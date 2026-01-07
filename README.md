delay_cat
=========


A delay cat, for bash script

<br>

## 0. Why delay_cat
case:
when we want handle event with common delay

```sh
cat event_fifo | while read event; do
  sleep 1; handle_event
done
```

//event N will delay N\*1 sec, not we want


```sh
cat event_fifo | while read event; do
  { sleep 1; handle_event; } &
done
```

//this may fork too many subshell


<br>

## 1. Use delay_cat

USAGE:
> cat input | delay_cat -d float_sec


```sh
cat event_fifo | delay_cat -d 1 | while read event; do
  handle_event
done
```

//delay_cat do the common delay  
//All input lines are delayed by a specified number of seconds.


<br>

## 2. Files

- py3 implement
- perl implement
- c implement


