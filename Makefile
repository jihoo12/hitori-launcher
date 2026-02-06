# 컴파일러 설정
CC = gcc

# pkg-config를 이용한 플래그 설정
CFLAGS = $(shell pkg-config --cflags gtk4 gtk4-layer-shell-0)
LIBS = $(shell pkg-config --libs gtk4 gtk4-layer-shell-0)

# 타겟 실행 파일 이름
TARGET = widget

# 소스 파일
SRC = main.c

# 기본 빌드 규칙
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LIBS)

# 생성된 빌드 파일 삭제
clean:
	rm -f $(TARGET)
