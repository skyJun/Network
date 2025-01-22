# Network
네트워크 관련 공부입니다

# 구글과 연결 되어 있는 거 확인
```zsh
tracerout 8.8.8.8
```
dns 구글 주소까지 가는 경로 추적
결과가 숫자 ip주소 이렇게 나올텐데 주소 값을 LAN대역이라 생각하면 된다

추적 결과볼때 *로 표시되는 경우가 있을텐데 이는 보안상 가려진 것

# 프로토콜 직접 확인
### wireshark 이용

TCP: Transmission Control Protocol

IPv4: Internet Protocol version 4

Ethernet

- 같이 통합해서 하나의 패킷으로 통신함

# 네트워크 계층 모델

## TCP/IP
### 1계층 네트워크 인터페이스
### 2계층 네트워크
### 3계층 전송
### 4계층 응용

## OSI 7계층
네트워크 표준으로 지정한 모델

### 1계층 물리
### 2계층 데이터 링크
이더넷
### 3계층 네트워크
ARP, IP, ICMP (오른쪽으로 갈수록 큰 계층)
### 4계층 전송
TCP, UDP
### 5계층 세션
### 6계층 표현
### 7계층 응용
HTTP

OSI는 통신 전반에 대한 표준, 
TCP/IP는 데이터 전송기술 특화

# 패킷
패킷이란 네트워크 상에서 전달되는 데이터를 통칭

데이터의 형식화된 블록

패킷은 제어정보와 사용자 데이터로 이루어져 있음

헤더 | 페이로드 | 풋터(잘 사용을 안함)
Ex. Ethernet | IPv4 | TCP | HTTP

HTTP앞에 헤더 TCP를 붙이고....
상위 계층에서 하위 계층으로 내려가면서 헤더를 붙임

이런 과정을 **캡슐화** 라고 함.

### 디캡슐화 (데이터를 받을 때)
하위 프로토콜부터 하나씩 하나씩 확인하여 데이터를 확인하는 과정

### 계층별 패킷의 이름

4계층의 Protocol Data Unit -> 세그먼트 (TCP까지 붙인 것)

3계층의 Protocol Data Unit -> 패킷

2계층의 Protoclo Data Unit -> 프레임


# Reference
[네트워크 기초 개정판](https://www.youtube.com/playlist?list=PL0d8NnikouEWcF1jJueLdjRIC4HsUlULi)

