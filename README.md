# z80cpu_emulater

Z80相当のエミュレータをC言語で作ります。

エミュレータ作りは初心者なので高速化のテクニックはありません。

研究用ですので実用度はありません。

2023/7/8現在で命令実装率は80%です。

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
$ time ./z80emu.out
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

# 命令テスト方法

codetest.cにテストコード記載してコンパイルする。

```
$ gcc codetest.c
$ ./a.out
```
