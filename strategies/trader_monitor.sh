find . \( -name "TimeBasedLog*.log" -o  -name  "my_exchange_fut_op_*.log" \)  -exec cat "{}"  \; |grep "\(.* e;.*\)\|\(.*Cancellation is time out.*\)"
