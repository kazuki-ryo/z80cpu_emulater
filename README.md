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

1. コンパイル
```
$ gcc -o z80emu.out z80emu.c
```

2. 実行
```
$ date;./z80emu.out;date
```

3. 実行結果
```
Tue Apr 11 14:24:19 JST 2023
z80 emulater v0.10
End of Executed z80.
AF   BC   DE   HL   IX   IY   SP   PC   AF2  BC2  DE2  HL2
0044 0000 0000 0000 0000 0000 ffff 0012 0000 0000 0000 0000
I  R  IFF1 IFF2
00 00 00   00
FLAG S,Z,Y,H,X,PV,N,C=01000100
PC[76 00 3e 00]
SP[00 11]
Tue Apr 11 14:24:49 JST 2023
```

※LD DE,2000の設定で実行した結果

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

655360回ループの実行結果比較。

|No.|環境|実行時間[sec]|Z80比率|Z80換算クロック|
|:--|:--|:--|:--|:--|
|1|MSXPEN MSX2+|5.6|-|3.58MHz|
|2|MSXPEN turboR ※1|0.86|6.6倍|23.64MHz|
|3|RaspberryPI pico※3|3.9|1.45倍|5.21MHz|
|4|Ubuntu(1.6GHz) ※1※2|0.15|37.86倍|135.56MHz|

※1 R800またはubuntuでは早すぎて計測できないため実行回数を増やして時間を求める。

※2 PC(Windows10,Pentium 4415Y 1.6GHz)のWSLのubuntuで実行。

※3 CPUは標準125MHz動作。

# 課題

- まだフラグ設定がおかしい。
- DAA命令の結果がおかしい。
- 未定義命令の実装。
- 高速化のため無駄を省いて最適化。

割り込みや周辺機器制御を省いているのにZ80の1.4倍相当は予想より低い。(MSX0を超えたい)
