#include <stdio.h>
#include <stdint-gcc.h>
#include <stdbool.h>
#include <asm-generic/errno.h>
#include "Types/Instructions.h"
#include "Types/Registers.h"
#include "Log/Logger.h"
#include "Types/Branches.h"
#include <string.h> 

/* existing prototypes … */
static int64_t executeStx(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal);

/* you already define this later in the file — declare it here so main() can call it */
static int32_t setBreakPoint(uint32_t memAddr);

/* helper prototypes for the CLI */
static int64_t run_free(void);
static int64_t run_to_breakpoint(void);
static int   is_breakpoint(uint32_t addr);
static void  dump_registers(void);
static void  dump_memory(uint32_t startAddr, uint32_t endAddr);
static void  print_menu(void);



#define DEFAULT_CODE_DIR  "/home/bruno-silva/Desktop/Simulator"
#define DEFAULT_CODE_FILE "testCode.txt"

#define IS_BIT_SET(x, n) (((x) >> (n)) & 0b1)

#define MAX_BREAK_POINTS 16

#define CODE_MEM_SIZE_BYTES 4096U

#define DATA_MEM_SIZE_WORDS 1024U

#define INT_CTRL_START      (DATA_MEM_SIZE_WORDS)
#define INT_CTRL_SIZE_WORDS 4U
#define INT_CTRL_END        (INT_CTRL_START + INT_CTRL_SIZE_WORDS)

#define GPIO_START          (INT_CTRL_END)
#define GPIO_SIZE_WORDS     4U
#define GPIO_END            (GPIO_START + GPIO_SIZE_WORDS)

#define UART_START          (GPIO_END)
#define UART_SIZE_WORDS     4U
#define UART_END            (UART_START + UART_SIZE_WORDS)

#define PS2_START           (UART_END)
#define PS2_SIZE_WORDS      4U
#define PS2_END             (PS2_START + PS2_SIZE_WORDS)

#define RESERVED            8U
#define TIMER_SIZE_WORDS    8U

#define CODE_MEM_SIZE_WORDS (CODE_MEM_SIZE_BYTES / 4U)

static uint8_t codeMem[CODE_MEM_SIZE_BYTES] = {0};
static uint32_t dataMem[DATA_MEM_SIZE_WORDS] = {0};

static uint32_t regFile[REG_NONE] = {0};
static uint32_t programCounter = 0;

static uint32_t breakpointList[MAX_BREAK_POINTS] = {0};
static size_t nofBreakpoints = 0;

static uint8_t zeroFlag = 0;
static uint8_t carryFlag = 0;
static uint8_t negativeFlag = 0;
static uint8_t overflowFlag = 0;

static bool dumpReg;

static int64_t executeNextInstruction();
static void updateFlags(uint32_t leftOp, uint32_t rightOp, uint64_t resVal, uint8_t isSub);
static int64_t loadCodeFile(const char* srcFile);

static int64_t executeNop(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal);
static int64_t executeAdd(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal);
static int64_t executeSub(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal);
static int64_t executeOr(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal);
static int64_t executeAnd(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal);
static int64_t executeNot(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal);
static int64_t executeXor(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal);
static int64_t executeCmp(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal);
static int64_t executeBxx(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal);
static int64_t executeJmp(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal);
static int64_t executeLd(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal);
static int64_t executeLdi(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal);
static int64_t executeLdx(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal);
static int64_t executeSt(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal);
static int64_t executeStx(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal);

static uint32_t readMem(uint32_t memAddr);
static void writeMem(uint32_t memAddr, uint32_t memVal);

static int64_t (*executeLut[])(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal) =
{
    executeNop,
    executeAdd,
    executeSub,
    executeOr,
    executeAnd,
    executeNot,
    executeXor,
    executeCmp,
    executeBxx,
    executeJmp,
    executeLd,
    executeLdi,
    executeLdx,
    executeSt,
    executeStx
};

int main(int argc, char** argv)
{
    int64_t ret;
    char defaultPath[512];

    // Decide which input file to load
    const char* srcPath = NULL;
    if (argc >= 2) {
        srcPath = argv[1];
    } else {
        snprintf(defaultPath, sizeof(defaultPath), "%s/%s",
                 DEFAULT_CODE_DIR, DEFAULT_CODE_FILE);
        srcPath = defaultPath;
    }

    ret = loadCodeFile(srcPath);
    if (ret < 0) {
        LOG_ERROR("Failed to load code file '%s' (err=%ld)\n", srcPath, ret);
        return 1;
    }

    dumpReg = false;
    programCounter = 0;

    // Simple CLI loop
    for (;;) {
        print_menu();

        char line[128];
        if (!fgets(line, sizeof(line), stdin)) break;

        unsigned opt = 0;
        if (sscanf(line, "%u", &opt) != 1) continue;

        switch (opt) {
            case 1:  // Free run
                run_free();
                break;

            case 2:  // Run to next breakpoint
                run_to_breakpoint();
                break;

            case 3: { // Set breakpoint: "3 <AddressHex>"
                uint32_t addr = 0;
                if (sscanf(line, "%*u %x", &addr) == 1) {
                    int32_t sret = setBreakPoint(addr);
                    if (sret == 0) {
                        printf("Breakpoint set at 0x%08x\n", addr);
                    } else {
                        printf("Failed to set breakpoint (err=%d)\n", sret);
                    }
                } else {
                    printf("Usage: 3 <AddressHex>\n");
                }
                break;
            }

            case 4:  // Dump registers
                dump_registers();
                break;

            case 5: { // Dump memory: "5 <StartHex> <EndHex>"
                uint32_t s = 0, e = 0;
                if (sscanf(line, "%*u %x %x", &s, &e) == 2) {
                    dump_memory(s, e);
                } else {
                    printf("Usage: 5 <StartHex> <EndHex>\n");
                }
                break;
            }

            case 6:  // Exit
                return 0;

            default:
                break;
        }
    }

    return 0;
}


static int64_t loadCodeFile(const char* srcFile)
{
    if (!srcFile) return -EINVAL;

    FILE* f = fopen(srcFile, "r");
    if (!f) {
        LOG_ERROR("Could not open '%s'\n", srcFile);
        return -EIO;
    }

    int64_t lines = 0;

    while (1) {
        uint32_t codeAddr;
        uint32_t d[4];
        // Expected line format (all hex):  @ADDR B3 B2 B1 B0
        int n = fscanf(f, "@%x %x %x %x %x", &codeAddr, &d[0], &d[1], &d[2], &d[3]);
        if (n == EOF) break;
        if (n != 5) { // skip malformed line
            int c;
            while ((c = fgetc(f)) != '\n' && c != EOF) {}
            continue;
        }

        if (codeAddr + 3 >= CODE_MEM_SIZE_BYTES) {
            LOG_ERROR("Code address 0x%08x out of range (max %u)\n",
                      codeAddr, CODE_MEM_SIZE_BYTES - 1);
            fclose(f);
            return -EPERM;
        }

        // little-endian assemble
        codeMem[codeAddr + 0] = (uint8_t)d[3];
        codeMem[codeAddr + 1] = (uint8_t)d[2];
        codeMem[codeAddr + 2] = (uint8_t)d[1];
        codeMem[codeAddr + 3] = (uint8_t)d[0];

        ++lines;
    }

    fclose(f);
    LOG_DEBUG("Loaded %ld instruction words from '%s'\n", (long)lines, srcFile);
    return 0;
}

static uint32_t signExtend(uint32_t x, uint32_t n)
{
    uint32_t mask = 1U << (n - 1);
    return (x ^ mask) - mask;
}

static int64_t executeNextInstruction()
{
    if (programCounter >= CODE_MEM_SIZE_BYTES)
    {
        LOG_ERROR("Program counter would exceed code memory size! Resetting CPU\n");
        return -EPERM;
    }

    uint32_t currentInstruction = (*(uint32_t*)&codeMem[programCounter]);
    opcode_et currentOpcode = (opcode_et) (currentInstruction >> 27);
    LOG_DEBUG("Executing OpCode: %d\n", currentOpcode);

    if (currentOpcode >= OP_STX)
    {
        if (currentOpcode != OP_HALT)
            LOG_ERROR("Invalid Opcode! Resetting CPU\n");
        else
        {
            LOG_ERROR("HALT found! Stopping CPU!\n");
            for (uint8_t i = 0; i < 32; ++i)
            {
                LOG_DEBUG("R%d -> %u\n", i, regFile[i]);
            }
        }
        return -EPERM;
    }

    register_et regDest = (register_et) ((currentInstruction >> 22) & 0b11111);
    register_et regLeft = (register_et) ((currentInstruction >> 17) & 0b11111);
    register_et regRight = (register_et) ((currentInstruction >> 11) & 0b11111);
    bool isImed = (currentInstruction >> 16) & 0b1;

    uint32_t imedVal = 0;
    if (currentOpcode <= OP_CMP)
    {
        imedVal = signExtend(currentInstruction & 0b1111111111111111, 16);
    }
    else if ((currentOpcode < OP_LDX) || (currentOpcode == OP_ST))
    {
        imedVal = signExtend(currentInstruction & 0b1111111111111111111111, 22);
    }
    else
    {
        imedVal = signExtend(currentInstruction & 0b11111111111111111, 17);
    }

    return executeLut[currentOpcode](regDest, regLeft, regRight, isImed, imedVal);
}

static int64_t executeAdd(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal)
{
    uint64_t tempVal;
    uint32_t rightOp;

    rightOp = isImed ? imedVal : regFile[regRight];
    tempVal = regFile[regLeft] + rightOp;
    updateFlags(regFile[regLeft], rightOp, tempVal, false);
    regFile[regDest] = tempVal;

    return programCounter + 4U;
}

static int64_t executeSub(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal)
{
    uint64_t tempVal;
    uint32_t rightOp;

    rightOp = isImed ? imedVal : regFile[regRight];
    tempVal = regFile[regLeft] - rightOp;
    updateFlags(regFile[regLeft], rightOp, tempVal, true);
    regFile[regDest] = tempVal;

    return programCounter + 4U;
}

static int64_t executeOr(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal)
{
    uint64_t tempVal;
    uint32_t rightOp;

    rightOp = isImed ? imedVal : regFile[regRight];
    tempVal = regFile[regLeft] | rightOp;
    updateFlags(regFile[regLeft], rightOp, tempVal, false);
    regFile[regDest] = tempVal;

    return programCounter + 4U;
}

static int64_t executeAnd(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal)
{
    uint64_t tempVal;
    uint32_t rightOp;

    rightOp = isImed ? imedVal : regFile[regRight];
    tempVal = regFile[regLeft] & rightOp;
    updateFlags(regFile[regLeft], rightOp, tempVal, false);
    regFile[regDest] = tempVal;

    return programCounter + 4U;
}

static int64_t executeNot(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal)
{
    uint64_t tempVal = ~regFile[regLeft];
    updateFlags(regFile[regLeft], 0, tempVal, false);
    regFile[regDest] = tempVal;

    return programCounter + 4U;
}

static int64_t executeXor(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal)
{
    uint64_t tempVal;
    uint32_t rightOp;

    rightOp = isImed ? imedVal : regFile[regRight];
    tempVal = regFile[regLeft] ^ rightOp;
    updateFlags(regFile[regLeft], rightOp, tempVal, false);
    regFile[regDest] = tempVal;

    return programCounter + 4U;
}

static int64_t executeCmp(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal)
{
    uint64_t tempVal;
    uint32_t rightOp;

    rightOp = isImed ? imedVal : regFile[regRight];
    tempVal = regFile[regLeft] - rightOp;
    updateFlags(regFile[regLeft], rightOp, tempVal, false);

    return programCounter + 4U;
}

static int64_t executeLd(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal)
{
    regFile[regDest] = readMem(imedVal);

    return programCounter + 4U;
}

static int64_t executeLdi(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal)
{
    regFile[regDest] = imedVal;

    return programCounter + 4U;
}

static int64_t executeLdx(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal)
{
    uint32_t targetAddress = regFile[regLeft] + imedVal;
    if (targetAddress >= DATA_MEM_SIZE_WORDS)
    {
        LOG_ERROR("Invalid data memory access! Resetting CPU!\n");
        return -EPERM;
    }

    regFile[regDest] = readMem(targetAddress);

    return programCounter + 4U;
}

static int64_t executeSt(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal)
{
    writeMem(imedVal, regFile[regDest]);

    return (int64_t) programCounter + 4U;
}

static int64_t executeStx(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal)
{
    uint32_t targetAddress = regFile[regLeft] + imedVal;

    writeMem(targetAddress, regFile[regDest]);

    return (int64_t) programCounter + 4U;
}

static int64_t executeNop(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal)
{
    return (int64_t) programCounter + 4U;
}

static int64_t executeJmp(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal)
{
    uint32_t targetAddress = regFile[regLeft] + imedVal;
    if (targetAddress >= CODE_MEM_SIZE_BYTES)
    {
        LOG_ERROR("Trying to jump to illegal address! Resetting CPU!\n");
        return -EPERM;
    }
    if (isImed)
        regFile[regDest] = programCounter;

    return (int64_t) targetAddress;
}

static int64_t executeBxx(register_et regDest, register_et regLeft, register_et regRight, bool isImed, uint32_t imedVal)
{
    uint32_t currentInstruction = (*(uint32_t*)&codeMem[programCounter]);
    int32_t branchOffset = (int32_t) signExtend(currentInstruction & 0b11111111111111111111111, 23);
    branch_cond_et branchCondition = (branch_cond_et) ((currentInstruction >> 23) & 0b1111);

    bool isBranchTaken = false;

    switch (branchCondition)
    {
        case COND_BRA:
            isBranchTaken = true;
            break;
        case COND_BNV:
            isBranchTaken = false;
            break;
        case COND_BCC:
            isBranchTaken = !carryFlag;
            break;
        case COND_BCS:
            isBranchTaken = carryFlag;
            break;
        case COND_BVC:
            isBranchTaken = !overflowFlag;
            break;
        case COND_BVS:
            isBranchTaken = overflowFlag;
            break;
        case COND_BEQ:
            isBranchTaken = zeroFlag;
            break;
        case COND_BNE:
            isBranchTaken = !zeroFlag;
            break;
        case COND_BGE:
            isBranchTaken = ((!negativeFlag) && (!overflowFlag)) || (negativeFlag && overflowFlag);
            break;
        case COND_BLT:
            isBranchTaken = (negativeFlag && (!overflowFlag)) || ((!negativeFlag) && overflowFlag);
            break;
        case COND_BGT:
            isBranchTaken = (!zeroFlag) && ((!negativeFlag && !overflowFlag) || (negativeFlag && overflowFlag));
            break;
        case COND_BLE:
            isBranchTaken = zeroFlag || ((negativeFlag && (!overflowFlag)) || ((!negativeFlag) && overflowFlag));
            break;
        case COND_BPL:
            isBranchTaken = !negativeFlag;
            break;
        case COND_BMI:
            isBranchTaken = negativeFlag;
            break;
        default:
            LOG_ERROR("Invalid branch condition received!\n");
    }

    if (isBranchTaken)
        return programCounter + branchOffset;

    return programCounter + 4;
}

static void updateFlags(uint32_t leftOp, uint32_t rightOp, uint64_t resVal, uint8_t isSub)
{
    overflowFlag = IS_BIT_SET(resVal, 32);
    carryFlag = IS_BIT_SET(resVal, 32);
    negativeFlag = IS_BIT_SET(resVal, 31);
    zeroFlag = resVal == 0;
}

static void writeMem(uint32_t memAddr, uint32_t memVal)
{
    if (memAddr < DATA_MEM_SIZE_WORDS)
    {
        dataMem[memAddr] = memVal;
    }
    else
    {
        LOG_ERROR("Un-handled memory address: 0x%x!\n", memAddr);
    }
}

static uint32_t readMem(uint32_t memAddr)
{
    uint32_t tempVal;

    if (memAddr < DATA_MEM_SIZE_WORDS)
        return dataMem[memAddr];

    if (memAddr == (UART_START + 0x03)) {
        uint32_t tempVal = 0;
        printf("Waiting on UART RX value...\n");
        fflush(stdout);
        if (scanf("%x", &tempVal) != 1) tempVal = 0;
        return tempVal;
    }


    LOG_ERROR("Un-handled memory address: 0x%x!\n", memAddr);
    return 0;
}


static int32_t setBreakPoint(uint32_t memAddr)
{
    if ((memAddr % 4) != 0)
        return -EINVAL;

    if (nofBreakpoints >= MAX_BREAK_POINTS)
        return -ENOBUFS;

    for (size_t i = 0; i < nofBreakpoints; ++i)
    {
        if (memAddr == breakpointList[nofBreakpoints])
        {
            LOG_ERROR("Trying to insert breakpoint at already set address!\n");
            return -EPERM;
        }
    }

    breakpointList[nofBreakpoints] = memAddr;
    nofBreakpoints++;

    return 0;
}

static int32_t showMenu()
{
    int32_t ret;
    uint32_t menuOption;
    char msgBuffer[128];

    printf("1. Free Run\n"
           "2. Goto next breakpoint\n"
           "3. Set breakpoint\n"
           "4. Dump registers\n"
           "5. Dump memory\n");

    fgets(msgBuffer, 128, stdin);
    ret = sscanf(msgBuffer, "%d", &menuOption);
    if (ret != 1)
        return -EPERM;
}
static void print_menu(void)
{
    printf("1. Free Run\n");
    printf("2. Goto next breakpoint\n");
    printf("3. Set breakpoint\n");
    printf("4. Dump registers\n");
    printf("5. Dump memory\n");
    printf("6. Exit\n");
}

static int is_breakpoint(uint32_t addr)
{
    for (size_t i = 0; i < nofBreakpoints; ++i) {
        if (breakpointList[i] == addr) return 1;
    }
    return 0;
}

static int64_t run_free(void)
{
    while (1) {
        int64_t ret = executeNextInstruction();
        if (ret < 0) return ret;       // HALT or error
        programCounter = (uint32_t)ret;
    }
}

static int64_t run_to_breakpoint(void)
{
    while (1) {
        if (is_breakpoint(programCounter)) {
            printf("Hit breakpoint at 0x%08x\n", programCounter);
            return 0;
        }
        int64_t ret = executeNextInstruction();
        if (ret < 0) return ret;       // HALT or error
        programCounter = (uint32_t)ret;
    }
}

static void dump_registers(void)
{
    for (uint8_t i = 0; i < 32; ++i) {
        printf("R%-2u -> %u\n", i, regFile[i]);
    }
    printf("Z=%u C=%u N=%u V=%u\n", zeroFlag, carryFlag, negativeFlag, overflowFlag);
}

static void dump_memory(uint32_t startAddr, uint32_t endAddr)
{
    if (endAddr < startAddr) {
        uint32_t t = startAddr; startAddr = endAddr; endAddr = t;
    }

    for (uint32_t a = startAddr; a <= endAddr; ++a) {
        if (a < DATA_MEM_SIZE_WORDS) {
            printf("%04x: %08x\n", a, dataMem[a]);
        } else {
            // avoid triggering UART scanf during memory dump
            printf("%04x: (peripheral / unmapped)\n", a);
        }
        if (a == 0xFFFFFFFF) break; // safety
    }
}
