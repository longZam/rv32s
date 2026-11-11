#!/usr/bin/env bash
#
# 사용법:
#   ./tests/run_test.sh [-v] [프로그램 경로]
#
#   프로그램 경로 기본값: <프로젝트 루트>/build/program
#   (환경변수 PROGRAM 으로도 지정 가능)
#   -v : 실패한 케이스의 diff 를 출력
#
# 각 tests/<case>/ 디렉토리에 아래 파일이 있어야 한다.
#   text.bin, data.bin          : 프로그램 인자 (이 순서로 전달)
#   expected_stdout.txt         : 기대하는 표준 출력
#   expected_data.out           : 기대하는 data.out 내용
#
# 프로그램은 작업 디렉토리에 data.out 을 생성하므로, 케이스마다
# 임시 디렉토리에서 실행하여 케이스 간 간섭 및 저장소 오염을 막는다.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

VERBOSE=0
if [[ "${1:-}" == "-v" ]]; then
    VERBOSE=1
    shift
fi

PROGRAM="${1:-${PROGRAM:-$ROOT_DIR/build/program}}"
# 상대 경로로 주어졌다면 현재 디렉토리 기준으로 절대 경로화
if [[ "$PROGRAM" != /* ]]; then
    PROGRAM="$(cd "$(dirname "$PROGRAM")" && pwd)/$(basename "$PROGRAM")"
fi

if [[ ! -x "$PROGRAM" ]]; then
    echo "오류: 실행 가능한 프로그램을 찾을 수 없습니다: $PROGRAM" >&2
    exit 2
fi

WORK_DIR="$(mktemp -d)"
trap 'rm -rf "$WORK_DIR"' EXIT

if [[ -t 1 ]]; then
    GREEN=$'\033[32m'; RED=$'\033[31m'; YELLOW=$'\033[33m'; RESET=$'\033[0m'
else
    GREEN=""; RED=""; YELLOW=""; RESET=""
fi

total=0
passed=0
failed_cases=()

for case_dir in "$SCRIPT_DIR"/*/; do
    [[ -d "$case_dir" ]] || continue
    case_dir="${case_dir%/}"
    name="$(basename "$case_dir")"
    total=$((total + 1))

    # 필수 파일 확인
    missing=()
    for f in text.bin data.bin expected_stdout.txt expected_data.out; do
        [[ -f "$case_dir/$f" ]] || missing+=("$f")
    done
    if (( ${#missing[@]} > 0 )); then
        echo "${RED}[FAIL]${RESET} $name (누락된 파일: ${missing[*]})"
        failed_cases+=("$name")
        continue
    fi

    run_dir="$WORK_DIR/$name"
    mkdir -p "$run_dir"

    # 프로그램 실행 (cwd = run_dir 이므로 data.out 은 run_dir 에 생성됨)
    (
        cd "$run_dir" || exit 255
        "$PROGRAM" "$case_dir/text.bin" "$case_dir/data.bin" \
            > "$run_dir/actual_stdout.txt" 2> "$run_dir/actual_stderr.txt"
    )
    exit_code=$?

    ok=1
    reasons=()

    # 표준 출력 비교
    if ! cmp -s "$run_dir/actual_stdout.txt" "$case_dir/expected_stdout.txt"; then
        ok=0
        reasons+=("stdout 불일치")
    fi

    # 파일 출력 비교
    if [[ ! -f "$run_dir/data.out" ]]; then
        ok=0
        reasons+=("data.out 이 생성되지 않음")
    elif ! cmp -s "$run_dir/data.out" "$case_dir/expected_data.out"; then
        ok=0
        reasons+=("data.out 불일치")
    fi

    if (( ok )); then
        passed=$((passed + 1))
        if (( exit_code != 0 )); then
            echo "${GREEN}[PASS]${RESET} $name ${YELLOW}(종료 코드: $exit_code)${RESET}"
        else
            echo "${GREEN}[PASS]${RESET} $name"
        fi
    else
        failed_cases+=("$name")
        echo "${RED}[FAIL]${RESET} $name: ${reasons[*]} (종료 코드: $exit_code)"
        if (( VERBOSE )); then
            if ! cmp -s "$run_dir/actual_stdout.txt" "$case_dir/expected_stdout.txt"; then
                echo "  --- stdout diff (expected vs actual) ---"
                diff "$case_dir/expected_stdout.txt" "$run_dir/actual_stdout.txt" | head -n 20 | sed 's/^/  /'
            fi
            if [[ -f "$run_dir/data.out" ]] && ! cmp -s "$run_dir/data.out" "$case_dir/expected_data.out"; then
                echo "  --- data.out 첫 차이 ---"
                cmp "$case_dir/expected_data.out" "$run_dir/data.out" 2>&1 | sed 's/^/  /'
            fi
            if [[ -s "$run_dir/actual_stderr.txt" ]]; then
                echo "  --- stderr ---"
                head -n 10 "$run_dir/actual_stderr.txt" | sed 's/^/  /'
            fi
        fi
    fi
done

echo
echo "결과: $passed / $total 통과"
if (( ${#failed_cases[@]} > 0 )); then
    echo "실패한 케이스: ${failed_cases[*]}"
    exit 1
fi
exit 0