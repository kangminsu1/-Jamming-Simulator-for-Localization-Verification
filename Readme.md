# Jamming Simulator for Localization Verification

---

## Overview

---
자율주행 시스템의 위치 추정 알고리즘이 얼마나 좋은 Performance를 가지는지 성능을 판단하기 위한 재밍 시뮬레이터입니다.

차량에 직접적으로 재밍을 인가함으로써, 타 차량에 위해를 가하지 않고 재밍을 발생시킬 수 있습니다.

Noise 재밍, 스푸핑, 미코닝 등이 작성되어 있으며, NMEA-0813 규격을 준수하는 GPS의 Latitude, Longitude, Height 데이터를 변환하는 방식입니다.

자세한 내용은 논문을 참고하시기 바랍니다.

[Paper](https://scienceon.kisti.re.kr/srch/selectPORSrchArticle.do?cn=JAKO202113855736872&dbt=NART)

[Copyright](https://www.cros.or.kr/psnsys/cmmn/infoPage.do?w2xPath=/ui/main/main.xml) 저작권 등록번호: C-2021-058946


## How to

---

사용자는 사전에 경로가 계획된 좌표가 담긴 .csv 파일을 user.cpp와 동일한 경로 내에 위치하여야 하며, 코드 내에서도 open할 .csv 파일 경로도 바꾸어주어야 한다. (Default: GPS_EX.csv)

또한, user.cpp는 Jamming_generator.cpp와 상호 TCP/IP로 통신할 수 있도록 상단에 IPAdress, SERVER_PORT를 기호에 맞게 바꾸어주어야 한다.

마지막으로, Jamming_monitering.py는 재밍을 육안으로 식별하는 코드이다.

※ 코드의 자세한 내용은 주석 참조 ※

### 1. (옵션) 모니터링 하기 위해 먼저 Jamming_monitering.py 실행

### 2. (필수) user.cpp 실행

### 3. (필수) Jamming_generator.cpp 실행

### 4. (필수) Jamming_generator.cpp의 Command 창에 나오는 설명대로 설정 및 조정
 
---
## Result
---

### Jamming
![Jamming](imgs/jamming.gif)

### Spoofing
![Spoofing](imgs/spoofing.gif)

### Meaconing
![Meaconing](imgs/meaconing.gif)
