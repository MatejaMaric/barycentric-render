srender: main.cpp
	g++ -o srender main.cpp
image.ppm: srender
	./srender
run: srender
	./srender
view: image.ppm
	feh -. image.ppm
clean:
	rm image.ppm
