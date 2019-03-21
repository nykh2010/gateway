# introduction

Bilink protocol and serial protocol used in BOE gateway with i.mx6ull platform, written with C. 

## tree

> ```powershell
> ├── Debug
> │   ├── config
> │   │   ├── config.ini
> │   │   └── serial.ini
> │   ├── example
> │   │   └── subdir.mk
> │   ├── makefile
> │   ├── objects.mk
> │   ├── serialserver.sock
> │   ├── source
> │   │   ├── bi_client
> │   │   │   └── subdir.mk
> │   │   ├── bilink
> │   │   │   └── subdir.mk
> │   │   ├── bi_server
> │   │   │   └── subdir.mk
> │   │   ├── entry
> │   │   │   └── subdir.mk
> │   │   ├── inifun
> │   │   │   └── subdir.mk
> │   │   ├── json
> │   │   │   └── subdir.mk
> │   │   ├── manage
> │   │   │   └── subdir.mk
> │   │   ├── ots
> │   │   │   └── subdir.mk
> │   │   ├── packet
> │   │   │   └── subdir.mk
> │   │   ├── rts
> │   │   │   └── subdir.mk
> │   │   ├── serial
> │   │   │   └── subdir.mk
> │   │   ├── subdir.mk
> │   │   ├── task
> │   │   │   └── subdir.mk
> │   │   └── timer
> │   │       └── subdir.mk
> │   └── sources.mk
> ├── doc
> │   ├── readme.md
> │   └── 网关MCU与SOC之间串口通讯测试.md
> ├── example
> │   ├── entry_test.c
> │   ├── entry_test.h
> │   ├── ots_test.c
> │   ├── ots_test.h
> │   ├── rts_test.c
> │   └── rts_test.h
> ├── make-for-arm.sh
> └── source
>  ├── bi_client
>  │   ├── bi_client.c
>  │   └── bi_client.h
>  ├── bilink
>  │   ├── bilink.c
>  │   ├── bilink.h
>  │   ├── bilink_packet.c
>  │   └── bilink_packet.h
>  ├── bi_server
>  │   ├── bi_server.c
>  │   └── bi_server.h
>  ├── config.h
>  ├── entry
>  │   ├── entry.c
>  │   └── entry.h
>  ├── entrydef.h
>  ├── inifun
>  │   ├── inirw.c
>  │   └── inirw.h
>  ├── main.c
>  ├── ots
>  │   ├── ots.c
>  │   └── ots.h
>  ├── packet
>  │   ├── frame.h
>  │   ├── packet.c
>  │   └── packet.h
>  ├── payload.h
>  ├── rts
>  │   ├── rts.c
>  │   └── rts.h
>  ├── serial
>  │   ├── serial.c
>  │   └── serial.h
>  ├── task
>  │   ├── task.c
>  │   └── task.h
>  └── timer
>      ├── timer.c
>      └── timer.h
> ```
>

## modified 2019-03-21

new frame:

```mermaid
graph LR
A(entry) --> B(bilink)
A --> C(server)
A --> D(client)
B --> E(serial send thread)
B --> F(serial recv thread)
C --> G(task json parse)
D --> H(serial msg parse)
```



































# Draw Diagrams With Markdown

 August 15, 2016 by typora.io 

Typora supports some Markdown extension for diagrams, you could enable this feature from preference panel.

When exporting as HTML, PDF, epub, docx, those rendered diagrams will  also be included, but diagrams features are not supported when  exporting markdown into other file formats in current version. Besides,  you should also notice that diagrams is not supported by standard  Markdown, CommonMark or GFM. Therefore, we still recommend you to insert  an image of these diagrams instead of write them in Markdown directly.

# Sequence

It is powered by [js-sequence](https://bramp.github.io/js-sequence-diagrams/), which would turn following code block into rendered diagrams:

~~~gfm
```sequence
Alice->Bob: Hello Bob, how are you?
Note right of Bob: Bob thinks
Bob-->Alice: I am good thanks!
```
~~~

![Snip20160816_1](https://support.typora.io/media/diagrams/Snip20160816_1.png)

Please refer [here](https://bramp.github.io/js-sequence-diagrams/#syntax) for syntax explanation.

# Flowchart

It is powered by [flowchart.js](http://flowchart.js.org/), which would turn following code block into rendered diagrams:

~~~gfm
```flow
st=>start: Start
op=>operation: Your Operation
cond=>condition: Yes or No?
e=>end

st->op->cond
cond(yes)->e
cond(no)->op
```
~~~

![Snip20160816_2](https://support.typora.io/media/diagrams/Snip20160816_2.png)

# Mermaid

Typora also has integration with [mermaid](https://knsv.github.io/mermaid/#mermaid), which supports sequence, flowchart and Gantt.

## Sequence

see [this doc](https://knsv.github.io/mermaid/#sequence-diagrams)

~~~gfm
```mermaid
%% Example of sequence diagram
  sequenceDiagram
    Alice->>Bob: Hello Bob, how are you?
    alt is sick
    Bob->>Alice: Not so good :(
    else is well
    Bob->>Alice: Feeling fresh like a daisy
    end
    opt Extra response
    Bob->>Alice: Thanks for asking
    end
```
~~~

![Snip20160816_3](https://support.typora.io/media/diagrams/Snip20160816_3.png)

## Flowchart

see [this doc](https://knsv.github.io/mermaid/#flowcharts-basic-syntax)

~~~gfm
```mermaid
graph LR
A[Hard edge] -->B(Round edge)
    B --> C{Decision}
    C -->|One| D[Result one]
    C -->|Two| E[Result two]
```
~~~

![Snip20160816_4](https://support.typora.io/media/diagrams/Snip20160816_4.png)

## Gantt

see [this doc](https://knsv.github.io/mermaid/#gant-diagrams)

~~~gfm
```mermaid
%% Example with selection of syntaxes
        gantt
        dateFormat  YYYY-MM-DD
        title Adding GANTT diagram functionality to mermaid

        section A section
        Completed task            :done,    des1, 2014-01-06,2014-01-08
        Active task               :active,  des2, 2014-01-09, 3d
        Future task               :         des3, after des2, 5d
        Future task2               :         des4, after des3, 5d

        section Critical tasks
        Completed task in the critical line :crit, done, 2014-01-06,24h
        Implement parser and jison          :crit, done, after des1, 2d
        Create tests for parser             :crit, active, 3d
        Future task in critical line        :crit, 5d
        Create tests for renderer           :2d
        Add to mermaid                      :1d

        section Documentation
        Describe gantt syntax               :active, a1, after des1, 3d
        Add gantt diagram to demo page      :after a1  , 20h
        Add another diagram to demo page    :doc1, after a1  , 48h

        section Last section
        Describe gantt syntax               :after doc1, 3d
        Add gantt diagram to demo page      : 20h
        Add another diagram to demo page    : 48h
```
~~~

![Snip20160816_5](https://support.typora.io/media/diagrams/Snip20160816_5.png)