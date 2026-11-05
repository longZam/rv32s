# RISC-V 시뮬레이터

지정된 RISC-V RV32I 명령어 14개를 실행하는 기능(ISA 수준) 시뮬레이터입니다.

## 1. 개발 환경

| 항목 | 내용 |
|------|------|
| 언어 | C |
| OS | [Ubuntu 24.04] 에서 실행 확인 |
| 외부 의존성 | 표준 라이브러리 |
| 참조 실행기 | 사용 안 함 |
---

네트워크·AI 서비스·대화형 입력에 의존하지 않습니다.

### Dev Container

이 저장소에는 `.devcontainer/` 설정이 포함되어 있어, Visual Studio Code의 Dev Containers로 열면 빌드 환경(컴파일러, CMake 등)이 자동으로 준비됩니다.

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

**표준 출력** (한 줄, 줄바꿈 포함, 소문자 8자리 16진수):

```
PC 0x00040014 invalid instruction 0x00000000
```

- PC는 invalid 명령어를 **fetch한 시점의 주소**이며 PC+4가 아닙니다.

**data.out**: `0x10000000` ~ `0x100003FF` 의 1,024 바이트를 주소 순서대로 저장한 바이너리 파일 (헤더·공백·줄바꿈 없음)

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
| 레지스터 | x0~x31 (32비트), x0은 항상 0, 초기값 전부 0 |
| 초기 PC / text 시작 | `0x00040000` |
| data 시작 | `0x10000000` (1,024 바이트) |
| 바이트 순서 | little-endian |
| 실행 한도 | 정상 명령어 최대 1,000,000개 |

### 종료 조건

1. 지원하지 않는 명령어가 fetch되면 **실행하지 않고** 종료 처리 (표준 출력 + `data.out` 저장)
2. 1,000,000개 실행 후 다음 fetch가 invalid이면 정상 종료
3. 1,000,000개 실행 후 다음 fetch가 지원 명령어이면 실행하지 않고 실행 한도 초과로 비정상 종료 [종료 코드: 1]

## 5. 테스트

### 전체 테스트 실행

```bash
bash tests/run_tests.sh
```

각 테스트 폴더의 `text.bin`, `data.bin`으로 시뮬레이터를 실행한 뒤, 표준 출력과 `data.out`을 기대 결과와 비교합니다.

### 포함된 테스트 항목

| 테스트 | 목적 |
|--------|------|
| `t01_basic` | 명세서 7절 예제 (lui, addi, add, sw) — 기대 `data.out` 앞 4바이트 `0c 00 00 00` |
| `t02_negative_imm` | 음수 즉시값, 부호 확장 |
| `t03_signed_cmp` | `blt`/`bge` 부호 비교 (`0xffffffff` vs `1`) |
| `t04_branch_offset` | 전방/후방 분기 PC 계산 |
| `t05_x0_fixed` | x0에 쓰기를 시도해도 0 유지 |
| `t06_overflow` | add/sub 32비트 래핑 |
| `t07_lw_sw` | lw/sw little-endian 및 data 영역 경계 |
| `t08_invalid_funct` | opcode는 맞지만 funct3/funct7이 다른 명령어 → invalid |
| `t09_empty_data` | `data.bin` 0바이트 → 1,024바이트 0 출력 |

## 6. 알려진 제한사항

- 명세에서 제외된 항목(예외, 시스템 호출, 비정렬 접근, 범위 밖 메모리 접근)은 유효 입력만 가정하며 별도 처리하지 않습니다.
- text segment는 읽기 전용입니다. text 영역에 대한 `sw`는 지원하지 않습니다.
