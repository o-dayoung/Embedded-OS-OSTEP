#! /usr/bin/env python
# -*- coding: utf-8 -*-

from __future__ import print_function
import sys
from optparse import OptionParser
import random
import math

# 유틸함수

def mustbepowerof2(bits, size, msg):
    """
    size가 2^bits인지 검사 (즉, size가 2의 거듭제곱인지 확인)
    예: size=16384이면 bits=14일 때 2^14=16384 -> OK
    """
    if math.pow(2, bits) != size:
        print('Error in argument: %s' % msg)
        sys.exit(1)

def mustbemultipleof(bignum, num, msg):
    """
    bignum이 num의 배수인지 검사
    (주소 공간 크기/물리 메모리 크기는 페이지 크기의 배수여야 함)
    """
    if (int(float(bignum) / float(num)) != (int(bignum) / int(num))):
        print('Error in argument: %s' % msg)
        sys.exit(1)

def convert(size):
    """
    "16k", "32m", "1g" 같은 문자열을 정수 바이트로 변환
    k=1024, m=1024^2, g=1024^3
    """
    length = len(size)
    lastchar = size[length - 1]

    if (lastchar == 'k') or (lastchar == 'K'):
        m = 1024
        nsize = int(size[0:length - 1]) * m
    elif (lastchar == 'm') or (lastchar == 'M'):
        m = 1024 * 1024
        nsize = int(size[0:length - 1]) * m
    elif (lastchar == 'g') or (lastchar == 'G'):
        m = 1024 * 1024 * 1024
        nsize = int(size[0:length - 1]) * m
    else:
        nsize = int(size)

    return nsize

# main program: 옵션 파밍

parser = OptionParser()

# -A: 접근할 가상 주소 목록을 직접 입력(콤마 구분). -1이면 랜덤 생성
parser.add_option('-A', '--addresses', default='-1',
                  help='a set of comma-separated pages to access; -1 means randomly generate',
                  action='store', type='string', dest='addresses')

# 랜덤 시드(결과 재현 가능)
parser.add_option('-s', '--seed', default=0,
                  help='the random seed',
                  action='store', type='int', dest='seed')

# 가상 주소 공간 크기
parser.add_option('-a', '--asize', default='16k',
                  help='address space size (e.g., 16, 64k, 32m, 1g)',
                  action='store', type='string', dest='asize')

# 물리 메모리 크기
parser.add_option('-p', '--physmem', default='64k',
                  help='physical memory size (e.g., 16, 64k, 32m, 1g)',
                  action='store', type='string', dest='psize')

# 페이지 크기
parser.add_option('-P', '--pagesize', default='4k',
                  help='page size (e.g., 4k, 8k, whatever)',
                  action='store', type='string', dest='pagesize')

# 생성할 가상 주소 개수
parser.add_option('-n', '--numaddrs', default=5,
                  help='number of virtual addresses to generate',
                  action='store', type='int', dest='num')

# 가상 페이지 중 valid로 만들 비율(%)
parser.add_option('-u', '--used', default=50,
                  help='percent of virtual address space that is used',
                  action='store', type='int', dest='used')

# verbose 모드(페이지 테이블 출력에 VPN 번호 표시)
parser.add_option('-v',
                  help='verbose mode',
                  action='store_true', default=False, dest='verbose')

# -c: 정답(주소 변환 결과)까지 계산해 출력
parser.add_option('-c',
                  help='compute answers for me',
                  action='store_true', default=False, dest='solve')

(options, args) = parser.parse_args()

# 입력 인자 출력

print('ARG seed',               options.seed)
print('ARG address space size', options.asize)
print('ARG phys mem size',      options.psize)
print('ARG page size',          options.pagesize)
print('ARG verbose',            options.verbose)
print('ARG addresses',          options.addresses)
print('')

random.seed(options.seed)

# 문자열 크기들을 바이트 정수로 변환
asize     = convert(options.asize)      # virtual address space size in bytes
psize     = convert(options.psize)      # physical memory size in bytes
pagesize  = convert(options.pagesize)   # page size in bytes
addresses = str(options.addresses)

# 기본검증

if psize <= 1:
    print('Error: must specify a non-zero physical memory size.')
    exit(1)

if asize < 1:
    print('Error: must specify a non-zero address-space size.')
    exit(1)

# 이 시뮬레이션은 단순화를 위해 "물리 메모리 > 주소 공간"만 허용
if psize <= asize:
    print('Error: physical memory size must be GREATER than address space size (for this simulation)')
    exit(1)

if psize >= convert('1g') or asize >= convert('1g'):
    print('Error: must use smaller sizes (less than 1 GB) for this simulation.')
    exit(1)

# 페이지 크기의 배수인지 확인
mustbemultipleof(asize, pagesize, 'address space must be a multiple of the pagesize')
mustbemultipleof(psize, pagesize, 'physical memory must be a multiple of the pagesize')

# 페이지 테이블 생성 준비

pages = int(psize / pagesize)      # 물리 프레임 개수
import array
used = array.array('i')            # 물리 프레임 사용 여부(중복 PFN 배정 방지용)
pt   = array.array('i')            # page table: VPN -> PFN 저장(유효하지 않으면 -1)

for i in range(0, pages):
    used.insert(i, 0)

vpages = int(asize / pagesize)     # 가상 페이지 개수(VPN 범위)

# 비트 수 계산(2의 거듭제곱이어야 log2가 딱 떨어짐)
vabits   = int(math.log(float(asize)) / math.log(2.0))     # VA 전체 비트 수
mustbepowerof2(vabits, asize, 'address space must be a power of 2')

pagebits = int(math.log(float(pagesize)) / math.log(2.0))  # offset 비트 수
mustbepowerof2(pagebits, pagesize, 'page size must be a power of 2')

vpnbits  = vabits - pagebits                                # VPN 비트 수
pagemask = (1 << pagebits) - 1                              # offset 마스크(하위 pagebits)

# VPN 마스크: offset 비트는 0, 그 위는 1이 되도록 (~pagemask)
# 파이썬의 ~는 무한비트로 뒤집기 때문에 32비트로 한번 제한(0xFFFFFFFF & ...)
vpnmask = 0xFFFFFFFF & ~pagemask


print('')
print('The format of the page table is simple:')
print('The high-order (left-most) bit is the VALID bit.')
print('  If the bit is 1, the rest of the entry is the PFN.')
print('  If the bit is 0, the page is not valid.')
print('Use verbose mode (-v) if you want to print the VPN # by')
print('each entry of the page table.')
print('')


# 각 VPN에 대해:
#  - options.used(%) 확률로 valid -> 임의의 PFN 하나 배정(중복 금지)
#  - 아니면 invalid -> PTE=0, pt에는 -1 저장

print('Page Table (from entry 0 down to the max size)')
for v in range(0, vpages):
    done = 0
    while done == 0:
        # 확률적으로 valid 할지 결정
        if ((random.random() * 100.0) > (100.0 - float(options.used))):
            # PFN 후보 하나 랜덤 선택
            u = int(pages * random.random())
            # 중복 PFN 배정 방지
            if used[u] == 0:
                used[u] = 1
                done = 1

                # verbose면 VPN 번호 같이 출력
                if options.verbose == True:
                    print('  [%8d]  ' % v, end='')
                else:
                    print('  ', end='')

                # PTE 출력: valid bit(0x80000000) + PFN(u)
                print('0x%08x' % (0x80000000 | u))

                # 실제로는 pt[vpn]에 PFN을 저장(유효하면 u)
                pt.insert(v, u)
        else:
            # invalid page
            if options.verbose == True:
                print('  [%8d]  ' % v, end='')
            else:
                print('  ', end='')
            print('0x%08x' % 0)

            # pt[vpn] = -1 로 저장(유효하지 않음을 의미)
            pt.insert(v, -1)
            done = 1
print('')

# 가상 주소 trace 생성

addrList = []
if addresses == '-1':
    # 랜덤으로 VA 생성
    for i in range(0, options.num):
        n = int(asize * random.random())
        addrList.append(n)
else:
    # 사용자가 준 목록을 그대로 사용
    addrList = addresses.split(',')

# 주소 변환 수행 및 출력

print('Virtual Address Trace')
for vStr in addrList:
    vaddr = int(vStr)

    if options.solve == False:
        # 문제 모드: 정답은 숨기고 "PA or invalid?"로 출력
        print('  VA 0x%08x (decimal: %8d) --> PA or invalid address?' % (vaddr, vaddr))
    else:
        # 정답 모드(-c): 실제 변환 결과까지 계산
        # 1) VPN, offset 분리
        vpn = (vaddr & vpnmask) >> pagebits

        # 2) 해당 VPN이 valid인지 확인
        if pt[vpn] < 0:
            print('  VA 0x%08x (decimal: %8d) -->  Invalid (VPN %d not valid)' % (vaddr, vaddr, vpn))
        else:
            # 3) PFN 얻고 물리 주소 계산
            pfn    = pt[vpn]
            offset = vaddr & pagemask
            paddr  = (pfn << pagebits) | offset

            print('  VA 0x%08x (decimal: %8d) --> %08x (decimal %8d) [VPN %d]' %
                  (vaddr, vaddr, paddr, paddr, vpn))
print('')

if options.solve == False:
    print('For each virtual address, write down the physical address it translates to')
    print('OR write down that it is an out-of-bounds address (e.g., segfault).')
    print('')

