# DDS 간단한 통신 예제

이 예제는 FastDDS를 이용한 간단한 발행자-구독자(Publisher-Subscriber) 통신 패턴을 보여줍니다.

## 빌드 방법

프로젝트 루트 디렉토리에서 다음 명령어를 실행하세요:

```bash
mkdir build
cd build
cmake ..
make
```

## 실행 방법

빌드 후 생성된 실행 파일은 `build/bin` 디렉토리에 있습니다.

터미널 두 개를 열고 각각 다음 명령어를 실행하세요:

1. 구독자(Subscriber) 시작:
```bash
./bin/subscriber
```

2. 발행자(Publisher) 시작:
```bash
./bin/publisher
```

## 동작 설명

발행자는 총 10개의 메시지를 초당 1개씩 발송합니다. 구독자는 메시지를 수신하고 메시지 내용과 인덱스를 출력합니다.

구독자는 10개의 메시지를 모두 수신한 후 자동으로 종료됩니다.

## 코드 설명

- `HelloWorld.idl`: DDS 데이터 타입을 정의합니다.
- `HelloWorldPublisher.cpp`: 발행자 구현 코드입니다.
- `HelloWorldSubscriber.cpp`: 구독자 구현 코드입니다.
- `CMakeLists.txt`: 빌드 설정 파일입니다. 