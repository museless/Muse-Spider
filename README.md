README
================================
The source code for MuseSp - Muse's Spider
--------------------------------

## <a name="index"/>Manual
* [Introduction](#Introduction)
* [Author](#Author)
* [Usage](#Usage)
* [Attention](#Attention)

## <a name="Introduction"/>Introduction
It's a limited web spider, it only catch html/htmls/htm, but you can alter the source code

## <a name="Author"/>Author
Author: Muse<br>
Email: 526659043@qq.com<br>

## <a name="Usage">Usage
MuseSp contain Six part

1. Urlbug ---	It's a function limited bug, you just can catch those website you put it at database.

2. Textbug ---	Limited for China news website, you just can use it to download the news based on the urls urlbug downloaded.

3. Extbug ---	Extract bug, used it to extract the news that textbug downloaded. Extracting to some words that you need. The word at the ../MuseSp/dict is a Economy-Based Dictionary.

4. Outbug ---	Output bug, it's useful bug for my need. If you want to have something new. Just write a new one. It use for output some statstics you need.

5. Kernel ---	It's a console, you can link the bug to kernel or not. But if you want the bug running day by day and you don't want to restart the bug. You can use Kernel. It can help you to remain the bug to change date.

6. Tools ---	Three different kinds of tools. The tools helps to create database needed table. Tool help create, change dictionary, I contain it at extbug's source, it's elf name called Mkdic. Tools that reinforce bug's function or use their data to do something.


## <a name="Attention"/>Attention
The mysql api I used in the source code's version is mysql-5.1.51.

