ESE 350: ProtoDrive 3.0

This is the official Github repository for the ProtoDrive 3.0 Project

Please visit our blog for more information about the project: protodrive3.blogspot.com

INTRODUCTION:
In the world of electric vehicles, there are many phases of development that
companies go through to produce working pieces of technology. Specifically,
when companies are testing the drive train control systems, they develop the
control system and test it using simulations of how they anticipate the vehicle
to behave and be used. After the completion of this testing phase, the control
system is loaded onto hardware and tested physically. It is at this stage that
problems usually arise, either due to the failure of the control system outright,
or due to a discrepancy between the software simulations and hardware imple-
mentation. Regardless, both the hardware and software team are forced to go
back to the drawing boards and edit the control software and hardware, only to
restart the process and risk running the same errors.

A way to remedy this issue is to create cheap hardware platform that is able to
simulate a control system on hardware without requiring the full implementation
of the vehicle. Rather than having to simulate the vehicles internal electronics,
the hardware platform would provide a cheap and quick way to test control sys-
tems without having to go through the tedious iterative process that companies
go through currently. Thus, ProtoDrive is born. Protodrive is limited in that
it cannot wholly replicate the complete hardware of the car. However, therein
lies the beauty of the solution. Protodrive is neither a full software testing plat-
form nor a full hardware testing platform; it resides somewhere in between. It
provides a hardware implementation on which to test code, but it also requires
the use of software to properly simulate the vehicle at hand to get relevant data
and statistics.

The ProtoDrive projet has been attempted in the past, and there currently exist
two successful iterations of the project that have each taken great strides.

The goals of this project are as follows:
  1. Recreate the ProtoDrive 2.0 platform from scratch
  2. Develop a control system for the mBed
    (a) Control System should properly boost and buck
    (b) Control system should take into account additional vehicle factors
        for power supply management
  3. Display the results of the control system simulations on a computer
  4. Create a presentable finished product for the system
  
The reach goals of this project were to:
  1. Create a PCB (Printed Circuit Board) for ProtoDrive
  2. Integrate Google Maps and Elevation API to get real location data
