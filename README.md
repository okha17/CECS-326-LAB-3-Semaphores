# CECS-326-LAB-3-Semaphores
Added semaphores to CECS 326 LAB 2: Concurrent Processing and Shared Memory. Modified swim_mill to include a semaphore to control access to critical resources such as the shared memory. Without semaphore movement of program is erratic with no locks and waits for access. 

•A set of pellet processes: Each pelletprocess drops a pellet at a random distance from the fish. We will call this process pellet. The pellet will start moving towards the fish with the flow of the river. For simplicity, we will assume that the pellet moves in a straight line downstream.

•A process called fish: the fish sees the pellet as it is dropped and moves one unit left or right to start aligning itself with the pellet. As the pellet moves towards the fish and the fish is at the same distance from the shore as the pellet, it stops changing its sideways position and devours the pellet when they are coincident. After eating the pellet, or if the pellet is missed, the fish returns to swimming midstream.

•A coordinator process named swim_mill: It is responsible for creating the fish process, the pelletprocesses, and coordinating the termination of pellets. We could have more than one pellet at anytime. Note that the position of pellet and fish are maintained by their respective processes.

To run the program use the makefile 


![image](https://user-images.githubusercontent.com/17859981/111240437-7c405680-85b8-11eb-8511-a3f76a1f8826.png)



Example output of program


![image](https://user-images.githubusercontent.com/17859981/111240507-9b3ee880-85b8-11eb-8da3-aab9330d4a30.png)
