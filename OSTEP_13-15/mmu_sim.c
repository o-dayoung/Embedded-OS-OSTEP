// mmu_sim.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PHYS_MEM_SIZE 65536   // 64KB 물리 메모리

// 1️⃣ 64KB 물리 메모리 생성
unsigned char physical_memory[PHYS_MEM_SIZE];

// TZASC 구조체 (Base & Bounds)
typedef struct {
    unsigned int base;    // 물리 메모리 시작 위치
    unsigned int bounds;  // 허용 크기
} TZASC;

// 2️⃣ 가상 → 물리 주소 변환 함수
unsigned int translate(TZASC config, unsigned int v_addr) {

    // bounds 검사
    if (v_addr >= config.bounds) {
        printf(" Security Fault! Virtual Address %u is Out of Bounds!\n", v_addr);
        exit(1);  // 3️⃣ 범위 초과 시 즉시 종료
    }

    unsigned int p_addr = config.base + v_addr;

    // 물리 메모리 전체 범위도 체크 (안전성 강화)
    if (p_addr >= PHYS_MEM_SIZE) {
        printf(" Physical Address Overflow!\n");
        exit(1);
    }

    return p_addr;
}

int main() {

    // Secure 영역: 32KB ~ 48KB (16KB 크기)
    TZASC secure_partition = {32768, 16384};

    printf("=== Base & Bounds MMU Simulation ===\n");

    // 정상 접근
    unsigned int v_addr1 = 100;
    unsigned int p_addr1 = translate(secure_partition, v_addr1);

    printf("Virtual Addr: %u → Physical Addr: %u\n", v_addr1, p_addr1);

    // 실제 메모리 쓰기
    physical_memory[p_addr1] = 0xAA;
    printf("Memory[%u] = 0x%X\n", p_addr1, physical_memory[p_addr1]);

    // 범위 초과 접근 (보안 폴트 발생)
    unsigned int v_addr2 = 20000;
    translate(secure_partition, v_addr2);

    return 0;
}