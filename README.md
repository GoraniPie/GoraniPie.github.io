# 個人用の静的ブログ生成ツール

* 本プロジェクトは、C++で作成したパーサー基盤HTML作成ツールです。<br>
日々の作文や学習記録を簡単に投稿・更新・削除できます。<br>
軽量型自己管理ブログとして運用されます。<br>
---
## 特徴
1. シンプルな構造
- `原文.txt` は `./original` ディレクトリにセーブ
- 投稿HTMLは `./post` に生成され、`index.html` に一覧が反映されます
- 静的構造なので、`github.io`として使えます。
2. 簡単な保守・制御
- 複雑なJSやテンプレートエンジンを使わず、C++パーサーによる完全制御
- パーサーを実行するたびに、`./post`と`index.html`が全体更新**されます


注意： `parser.cpp`は`C++17`以上のコンパイラが必須です。<br>

---
## 使用方法

1. `./original/writting`は作成物の原文のディレクトリです。新しい作成物を書きます。<br>
原文は以下の作成規則を従って書いてください。<br>
例：
`./original/writting/test1.txt`<br>
```
@file_name: use English to avoid directory name error
@title: タイトルです。
@date: 20250419
@tags: 作文

本文です。作文内容を書きます。

```
2. 原文の**変更・追加・削除**の時、ターミナルで`parser.cpp`を起動します。<br>
`g++ -std=c++17 -o parser parser.cpp ./parser`<br>
`parser.exe`
3. `parser.cpp`の起動の後、`./index.html`と`./post```が更新されます。
原文のファイルと`post_template.html`を合わせたHTMLフォーマットのファイルが`./post`に生成されます。
---
## カスタマイズ
- `post_template.html`の編集すれば、投稿の出力を自由に変えます。
- `index.html`のコメント`<!--writting_auto_add_start-->`と`<!--writting_auto_add_end-->`の範囲外は自由に修正できます。
### メモ
自分自身の日本語学習や日々の整理を継続できるように、扱いやすい・拡張しやすいプログラムを目指しました。<br>
市販のテンプレート等に頼らず、自分の記録スタイルや学習内容に合わせて、最小構成のブログ環境を構築しました。<br>