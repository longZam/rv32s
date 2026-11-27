# RISC-V 시뮬레이터

지정된 RISC-V RV32I 명령어 14개를 실행하는 기능(ISA 수준) 시뮬레이터입니다.

## 0. 팀원

팀 08

| 이름 | 학과 | 학번 |
|------|------|------|
| 권서림 | 정보보안암호수학과 | 20232073 |
| 권동욱 | 소프트웨어학부 | 20233019 |
| 송지민 | 인공지능학부 | 20243157 |
| 최승욱 | 소프트웨어학부 | 20223150 |


## 1. 개발 환경

| 항목 | 내용 |
|------|------|
| 언어 | C(C11),gcc 15.2.0(Ubuntu 26.04.1 기준) |
| 빌드 도구 | CMake 4.2.3(Ubuntu 26.04.1 기준) |
| OS | Ubuntu 24.04, Ubuntu 26.04.1(WSL) 에서 빌드 및 테스트 확인 |
| 외부 의존성 | C 표준 라이브러리, POSIX `<sysexits.h>`(종료 코드) |

---

### Dev Container

이 저장소에는 `.devcontainer/` 설정이 포함되어 있어, Visual Studio Code의 Dev Containers로 열면 빌드 환경(컴파일러, CMake 등)이 자동으로 준비됩니다.

### 파일 배치

```
rv32s/
|-- CMakeLists.txt        빌드 설정 (src/ 아래 .c 파일을 모아 program 실행 파일 생성)
|-- README.md
|-- include/
|   |-- simulator.h       시뮬레이터 구조체 정의와 함수 선언
|   `-- executors.h       명령어별 실행 함수(__add, __sw 등) 선언
|-- src/
|   |-- main.c            프로그램 시작점: 입력 파일 읽기, 실행, 결과 출력
|   |-- simulator.c       명령어 fetch, 해독, 실행 루프와 메모리 읽기/쓰기
|   `-- executors/
|       |-- r-format.c    add, sub, and, or
|       |-- i-format.c    addi, andi, ori, lw
|       |-- s-format.c    sw
|       |-- sb-format.c   beq, bne, blt, bge
|       `-- u-format.c    lui
`-- tests/
    |-- run_tests.sh      전체 테스트 실행 스크립트
    `-- t01 ~ t09 폴더    각 테스트의 text.bin, data.bin, 기대 출력
```   

## 2. 빌드

요구 사항: CMake 3.13 이상, C11 이상 지원 컴파일러

```bash
cmake -S . -B build
cmake --build build
```

---

## 3. 실행

```bash
./build/program <text.bin 경로> <data.bin 경로>
```

예시:

```bash
./build/program text.bin data.bin
```


- 인수는 **text.bin** 파일 경로, **data.bin** 파일 경로를 입력받습니다.
- `data.out` 은 **현재 작업 디렉터리**에 생성되며, 기존 파일이 있으면 덮어씁니다.
- 표준 출력에는 종료 메시지 한 줄만 출력합니다. 진단·디버그 정보는 표준 오류(stderr)로만 출력합니다.

### 입력 형식

- `text.bin`, `data.bin`: 헤더 없는 순수 바이너리 파일 (ASCII/16진수 텍스트 아님)

### 출력 형식

**표준 출력**:

```
PC 0x00040014 invalid instruction 0x00000000
```

**data.out**: `0x10000000` - `0x100003FF` 의 1,024 바이트를 주소 순서대로 저장한 바이너리 파일

---

## 4. 지원 범위

### 지원 명령어

| 형식 | 명령어 |
|------|--------|
| R | `add`, `sub`, `and`, `or` |
| I | `addi`, `andi`, `ori`, `lw` |
| S | `sw` |
| B | `beq`, `bne`, `blt`, `bge` |
| U | `lui` |

- 위 14개에 해당하지 않는 32비트 명령어는 **invalid instruction**으로 간주합니다.

### 실행 모델

| 항목 | 값 |
|------|----|
| 레지스터 | x0-x31 (32비트), x0은 항상 0, 초기값 전부 0 |
| 초기 PC / text 시작 | `0x00040000` |
| data 시작 | `0x10000000` (1,024 바이트) |
| 바이트 순서 | little-endian |
| 실행 한도 | 정상 명령어 최대 1,000,000개 |

### 종료 조건

1. 지원하지 않는 명령어가 fetch되면 **실행하지 않고** 종료 처리 (표준 출력 + `data.out` 저장)
2. 1,000,000개 실행 후 다음 fetch가 invalid이면 그대로 종료 처리
3. 1,000,000개 실행 후 다음 fetch가 지원 명령어여도 실행하지 않고 같은 형식으로 출력 후 종료 (종료 코드 0)

## 5. 테스트

### 전체 테스트 실행

```bash
./tests/run_tests.sh
```

각 테스트 폴더의 `text.bin`, `data.bin`으로 시뮬레이터를 실행한 뒤, 표준 출력과 `data.out`을 기대 결과와 비교합니다.
82개의 보조 테스트는 참조 실행기로 만든 것이며, 테스트 보조용으로만 사용했습니다. 자세한 내용은 보고서 11.에 기록했습니다.

### 포함된 테스트 항목

| 테스트 | 목적 |
|--------|------|
| `t01_basic_and_store` | 기본 연산과 메모리 쓰기 |
| `t02_randomized_1` | 무작위로 생성된 명령어 시퀀스 1 |
| `t03_randomized_2` | 무작위로 생성된 명령어 시퀀스 2 |
| `t04_invalid_ffffffff` | 정의되지 않은 명령어 종료 처리 |
| `t05_backward_signed_branch` | 음수 방향 분기 처리 |
| `t06_negimm_lui_sw` | addi 음수 즉시값 부호 확장, lui로 data 주소 생성, sw 4바이트 쓰기, invalid 종료 |
| `t07_x0_write_ignored` | x0 쓰기 무시 확인 (구현 오류 시 data 앞 4바이트가 01 00 00 00) |
| `t08_text_1024bytes` | text.bin이 정확히 1024바이트일 때의 입력 경계 조건 (수정 전 실패) |
| `t09_lw_sw_offset` | lw little-endian 읽기, sw 오프셋 확인 (x1 = 0x12345678) |

## 6. 알려진 제한사항

- 명세에서 제외된 항목(예외, 시스템 호출, 비정렬 접근, 범위 밖 메모리 접근)은 유효 입력만 가정하며 별도 처리하지 않습니다.
- text segment는 읽기 전용입니다. text 영역에 대한 `sw`는 지원하지 않습니다.