// Copyright (c) 2005 - 2017 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#include "rttrDefines.h" // IWYU pragma: keep
#include "GameManager.h"
#include "PlayerGameCommands.h"
#include "ReplayInfo.h"
#include "helpers/format.hpp"
#include "network/ClientInterface.h"
#include "network/GameClient.h"
#include "s25util/Log.h"

void GameClient::ExecuteGameFrame_Replay()
{
    AsyncChecksum checksum = AsyncChecksum::create(*game);

    const unsigned curGF = GetGFNumber();
    RTTR_Assert(replayinfo->next_gf >= curGF || curGF > replayinfo->replay.GetLastGF()); //-V807

    bool cmdsExecuted = false;
    // Execute all commands from the replay for the current GF
    while(replayinfo->next_gf == curGF)
    {
        // What type of command follows?
        Replay::ReplayCommand rc = replayinfo->replay.ReadRCType();

        if(rc == Replay::RC_CHAT)
        {
            uint8_t player, dest;
            std::string message;
            replayinfo->replay.ReadChatCommand(player, dest, message);

            if(ci)
                ci->CI_Chat(player, ChatDestination(dest), message);
        } else if(rc == Replay::RC_GAME)
        {
            cmdsExecuted = true;

            PlayerGameCommands msg;
            uint8_t gcPlayer;
            replayinfo->replay.ReadGameCommand(gcPlayer, msg);

            // Execute them
            ExecuteAllGCs(gcPlayer, msg);
            AsyncChecksum& msgChecksum = msg.checksum;

            // Check for async if checksum data is valid
            if(msgChecksum.randChecksum != 0 && msgChecksum != checksum)
            {
                // Show message if this is the first async GF
                if(replayinfo->async == 0)
                {
                    if(ci)
                    {
                        ci->CI_ReplayAsync(
                          helpers::format(_("Warning: The played replay is not in sync with the original match. (GF: %u)"), curGF));
                    }

                    LOG.write("Async at GF %u: Checksum %i:%i ObjCt %u:%u ObjIdCt %u:%u\n") % curGF % msgChecksum.randChecksum
                      % checksum.randChecksum % msgChecksum.objCt % checksum.objCt % msgChecksum.objIdCt % checksum.objIdCt;

                    // and pause the game for further investigation
                    framesinfo.isPaused = true;
                    if(skiptogf)
                        skiptogf = 0;
                }

                replayinfo->async++;
            }
        }
        // Read GF of next command
        replayinfo->replay.ReadGF(&replayinfo->next_gf);
    }

    // Run game simulation
    NextGF(cmdsExecuted);

    // Check for game end
    if(curGF == replayinfo->replay.GetLastGF())
    {
        if(ci)
        {
            using std::chrono::duration_cast;
            std::chrono::seconds runtime = duration_cast<std::chrono::seconds>(GAMEMANAGER.GetRuntime());
            std::chrono::hours hours = duration_cast<std::chrono::hours>(runtime);
            std::chrono::minutes mins = duration_cast<std::chrono::minutes>(runtime - hours);
            std::chrono::seconds secs = duration_cast<std::chrono::seconds>(runtime - hours - mins);
            const std::string text =
              (boost::format(_("Notice: The played replay has ended. (GF: %u, %dh %dmin %ds, TF: %u, AVG_FPS: %u)")) % curGF % hours.count()
               % mins.count() % secs.count() % GAMEMANAGER.GetNumFrames() % GAMEMANAGER.GetAverageGFPS())
                .str();

            ci->CI_ReplayEndReached(text);
        }

        if(replayinfo->async != 0)
        {
            // Messenger im Game
            if(ci)
            {
                ci->CI_ReplayEndReached(helpers::format(_("Notice: Overall asynchronous frame count: %u"), replayinfo->async));
            }
        }

        replayinfo->end = true;
        framesinfo.isPaused = true;
        if(skiptogf)
            skiptogf = GetGFNumber();
    }
}
