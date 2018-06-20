The only code that matters:

_2018_control_code_v2 -> this is the code that controlled the turbine during the competition
  -Functional overview: change pitch to optimize power output until voltage went over 39, in which case change the pitch angle to drop the voltage (to avoid DQ). Read the states of the kill switch and pcc-disconnect sending a signal to the load arduino when it is braked.

The durability test code -> this was ran during the durability_test at the competition
  -Try to maintain voltage at 5V brake if voltage gets to high. Fail miserably because the circuit that controlled voltage broke.

_2018_load_code -> this controlled the load arduino during the competition
  -Functional overview: read the state sent from the control arduino and turn on the switch to power the control arduino if the system is braked
