#pragma once

int BetterNetworkingLikeMineCraft()
{

	return 0;
}

/*
	THE GOAL:
	SIMPLE MULTIPLAYER, WHERE SERVER RUNS AT 60TPS(GAME LOGIC ONLY)

	THREADS USED:
		-SERVER:
			-MAIN THREAD
			-NETWORKING THREAD

		-CLIENT:
			-MAIN THREAD
			-NETWORKING THREAD

	NETWORKING MODEL:
		-

	TICK MODEL:
		PSUDO CODE:
			-CLIENT:
				{
					MAIN THREAD:
						while(running)
						{
							processInput();

							while(timeForTick())
							{
							    tick();
							}

							render();
						}
						void tick()
						{
							processPackets();
						
							updatePlayer();
						
							updateEntities();
						
							updatePhysics();
						}
				}

				-SERVER:
					{
						while(running)
						{
						    waitUntilNextTick();
						
						    tick();
						}
						void tick()
						{
							processPackets();
						
							updatePlayers();
						
							updateMobs();
						
							updatePhysics();
						
							sendSnapshots();
						}
					}
*/