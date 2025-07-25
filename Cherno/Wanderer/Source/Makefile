CFLAGS = -std=c++23
LDFLAGS = -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

GAMESOURCE = wanderer.cpp
MMAKERSOURCE = wandererMapMaker.cpp
GAMETITLE = ../Wanderer
MMAKERTITLE = ../WandererMapMaker

.PHONY: debug

debug:
	g++ -O0 -static $(CFLAGS) -o $(MMAKERTITLE) $(MMAKERSOURCE)
	chmod +x $(MMAKERTITLE)
	./$(MMAKERTITLE)


release:
	g++ -O3 -static -Wno-unused-result $(CFLAGS) -o $(MMAKERTITLE) $(MMAKERSOURCE)
	chmod +x $(MMAKERTITLE)
	./$(MMAKERTITLE)

clean:
	rm -f $(MMAKERTITLE)

