
## test

DELAY_CMD="delay_cat.aio.py -d 1 "
DELAY_CMD="delay_cat.py -d 1 "
DELAY_CMD="delay_cat.pl -d 1 "
DELAY_CMD="delay_cat    -d 1 "

test1(){
for i in `seq -w 99`; do echo "$i `date +"%Y-%m-%d %H:%M:%S.%3N"`"; sleep 0.01; done \
| $DELAY_CMD \
| awk '{print $0; system("echo \033[033m"$1" `date +%Y-%m-%d\\ %H:%M:%S.%3N`\033[0m")}'
}

test2(){
for i in `seq -w 99`; do echo "$i `date +"%Y-%m-%d %H:%M:%S.%3N"`"; sleep 0.5; done \
| $DELAY_CMD \
| awk '{print $0; system("echo \033[033m"$1" `date +%Y-%m-%d\\ %H:%M:%S.%3N`\033[0m")}'
}

## note

: <'

- py aio 版受 event loop 调度波动
- py select 版 时间更精准
- perl 少点依赖
- c 更少的依赖

'

