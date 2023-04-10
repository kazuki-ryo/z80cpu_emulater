# z80cpu_emulater

Z80相当のエミュレータをC言語で作ります。

エミュレータ作りは初心者なので高速化のテクニックはありません。

研究用ですので実用度はありません。

2023/4/10現在で命令実装率は47%です。

# 特徴

以下は予定も含みます。

- Z80/R800相当の命令サポート
- メモリ64KBサポート、将来的にMMU（Memory mapper)サポート
- 演算と表示ユニットは独立

# コンパイル方法

Windows上のubuntuでは以下のコマンドを実行する。
```
gcc -o z80emu.out z80emu.c
```

Raspberrypi pico用実行方法は準備中。


# ベンチマーク

自作エミュレータの速度が如何ほどか気になるのでテスト。
テストコードは以下の通り。
```
org 0c000h
LD DE,10
LOOP1:
LD BC,0
LOOP2:
DEC BC
LD A,B
OR C
JP NZ,LOOP2
DEC DE
LD A,D
OR E
JP NZ,LOOP1
RET
```

msxpenのBASICコードは以下を記載して実行する。
```
10 clear200,&hbfff
20 bload"program.bin"
30 defusr=&hc000
35 time=0
40  a=usr(0):t=time
50 print t/60

```

実行結果。

|No.|環境|実行時間|Z80比率|Z80換算クロック|
|:--|:--|:--|:--|:--|
|1|MSXPEN MSX2+|5.6|1|3.58MHz|
|2|MSXPEN turboR|0.86|6.6|23.64MHz|
|3|RaspberryPI pico|3.9|1.45|5.21MHz|
|4|Ubuntu(1.6GHz)|0.15|37.86|135.56MHz|

※R800またはubuntuでは早すぎて計測できないため実行回数を増やして時間を求める。
