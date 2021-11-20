# MovingFunction
A Simple (Self-Modifying) Function That Copies Itself Around in Memory

From MovingFunction.cpp :



    11/16/2021 - Zachary Washburn - zachary.washburn.business@gmail.com
    
    This is a simple function that will move itself around in memory by allocating new memory, copying itself over,
    and running itself there. DEBUG mode will print out the new base address of the function, and you can make it
    free its previous instances by defining FREE_PREVIOUS_MEMORY.

    Everything is calculated using rel offsets so you should be able to add whatever code you would like in-between, as
    long as you preserve the registers and stack!

    You will need to set the .text section to have PAGE_EXECUTE_READWRITE rights, aswell as turn of Data Execution Prevention
    for this code to work. with a bit of changes you could just not modify the first instance (no .text modification) and
    call VirtualProtect to not have these requirements.



