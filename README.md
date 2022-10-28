# Assembler, Linker, Loader
SIC-XE machine based Assembler, Linker, Loader

## Duration
> 2021.03. ~ 2021.06.

## About Project
### Project 1. Shell
>SIC/XE머신 구현을 위한 전 단계로서 어셈블러, 링커, 로더들을 실행하게 될 셸(shell)과 컴파일을 통해서 만들어진 object코드가 적재되고 실행될 메모리공간, mnemonic (ADD, COMP, FLOAT, etc …)을 opcode값으로 변환하는 OPCODE 테이블과 관련 명령어들을 구현하는 프로그램
### Project 2. Assembler
> 프로젝트1에서 구현한 shell에 assemble 기능을 추가하는 프로그램을 작성하는 프로젝트
> SIC/XE machine의 assembly program source 파일을 입력 받아서 object파일과 list 파일을 생성하고 어셈블 과정 중 생성된 symbol table과 결과물인 object 파일을 볼 수 있는 기능을 제공
### Project 3. Linker & Loader
> 프로젝트 1, 2에서 구현한 쉘(shell)에 linking과 loading 기능을 추가하는 프로그램을 생성
> 프로젝트 2에서 구현된 assemble 명령을 통해 생성된 object 파일을 link시켜 메모리에 올리는 작업을 수행하는 프로그램

## Technology Stack
C, Data Structure(linked list), Hash
