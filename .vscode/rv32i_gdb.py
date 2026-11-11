import gdb

PRINTER_TYPE_NAMES = {"uint32_t"}

R = ["zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1",
     "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",
     "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
     "t3", "t4", "t5", "t6"]


def sext(v, bits):
    v &= (1 << bits) - 1
    return v - (1 << bits) if v & (1 << (bits - 1)) else v


def disasm(w, pc=None):
    w &= 0xFFFFFFFF
    op = w & 0x7F
    rd = R[(w >> 7) & 31]
    f3 = (w >> 12) & 7
    rs1 = R[(w >> 15) & 31]
    rs2 = R[(w >> 20) & 31]
    f7 = w >> 25
    imm_i = sext(w >> 20, 12)

    def tgt(off):
        return f"{off:+d}" if pc is None else f"0x{(pc + off) & 0xFFFFFFFF:x}"

    if w == 0x00000013:
        return "nop"
    if op == 0x37:
        return f"lui {rd}, 0x{w >> 12:x}"
    if op == 0x17:
        return f"auipc {rd}, 0x{w >> 12:x}"
    if op == 0x6F:
        imm = ((w >> 31) << 20) | (((w >> 12) & 0xFF) << 12) | \
              (((w >> 20) & 1) << 11) | (((w >> 21) & 0x3FF) << 1)
        return f"jal {rd}, {tgt(sext(imm, 21))}"
    if op == 0x67 and f3 == 0:
        return f"jalr {rd}, {imm_i}({rs1})"
    if op == 0x63:
        imm = ((w >> 31) << 12) | (((w >> 7) & 1) << 11) | \
              (((w >> 25) & 0x3F) << 5) | (((w >> 8) & 0xF) << 1)
        n = {0: "beq", 1: "bne", 4: "blt", 5: "bge", 6: "bltu", 7: "bgeu"}.get(f3)
        if n:
            return f"{n} {rs1}, {rs2}, {tgt(sext(imm, 13))}"
    if op == 0x03:
        n = {0: "lb", 1: "lh", 2: "lw", 4: "lbu", 5: "lhu"}.get(f3)
        if n:
            return f"{n} {rd}, {imm_i}({rs1})"
    if op == 0x23:
        n = {0: "sb", 1: "sh", 2: "sw"}.get(f3)
        if n:
            imm = sext((f7 << 5) | ((w >> 7) & 31), 12)
            return f"{n} {rs2}, {imm}({rs1})"
    if op == 0x13:
        if f3 == 1 and f7 == 0:
            return f"slli {rd}, {rs1}, {(w >> 20) & 31}"
        if f3 == 5 and f7 in (0, 0x20):
            n = "srli" if f7 == 0 else "srai"
            return f"{n} {rd}, {rs1}, {(w >> 20) & 31}"
        n = {0: "addi", 2: "slti", 3: "sltiu", 4: "xori", 6: "ori", 7: "andi"}.get(f3)
        if n:
            return f"{n} {rd}, {rs1}, {imm_i}"
    if op == 0x33:
        if f7 == 0x01:  # M extension
            n = ["mul", "mulh", "mulhsu", "mulhu", "div", "divu", "rem", "remu"][f3]
            return f"{n} {rd}, {rs1}, {rs2}"
        if f7 == 0x00:
            n = ["add", "sll", "slt", "sltu", "xor", "srl", "or", "and"][f3]
            return f"{n} {rd}, {rs1}, {rs2}"
        if f7 == 0x20 and f3 in (0, 5):
            n = "sub" if f3 == 0 else "sra"
            return f"{n} {rd}, {rs1}, {rs2}"
    if op == 0x0F:
        return "fence.i" if f3 == 1 else "fence"
    if op == 0x73:
        if w == 0x00000073:
            return "ecall"
        if w == 0x00100073:
            return "ebreak"
        n = {1: "csrrw", 2: "csrrs", 3: "csrrc", 5: "csrrwi", 6: "csrrsi", 7: "csrrci"}.get(f3)
        if n:
            src = str((w >> 15) & 31) if f3 >= 5 else rs1
            return f"{n} {rd}, 0x{w >> 20:x}, {src}"
    return f".word 0x{w:08x}"


class RvDis(gdb.Function):
    """$rvdis(word [, pc]) -> 디스어셈블 문자열"""

    def __init__(self):
        super().__init__("rvdis")

    def invoke(self, word, pc=None):
        w = int(word)
        # struct로 감싼 경우 첫 필드를 사용
        return disasm(w, int(pc) if pc is not None else None)


RvDis()


class InsnPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        v = self.val
        if v.type.strip_typedefs().code == gdb.TYPE_CODE_STRUCT:
            v = v[v.type.strip_typedefs().fields()[0].name]
        w = int(v) & 0xFFFFFFFF
        return f"0x{w:08x}  {disasm(w)}"


def lookup(val):
    if val.type.unqualified().name in PRINTER_TYPE_NAMES:
        return InsnPrinter(val)
    return None


gdb.pretty_printers.append(lookup)
print("rv32i_gdb loaded: $rvdis(word[, pc]), pretty-printer for", sorted(PRINTER_TYPE_NAMES))