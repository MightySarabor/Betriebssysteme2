# Verwende ein GCC-Image mit ZeroMQ-Unterstützung
FROM gcc:latest

# Installiere ZeroMQ und Entwicklungswerkzeuge
RUN apt-get update && apt-get install -y libzmq3-dev

# Setze das Arbeitsverzeichnis
WORKDIR /app

# Kopiere die Dateien in den Container
COPY server.c client.c .

# Kompiliere beide Programme
RUN gcc -o server server.c -lzmq
RUN gcc -o client client.c -lzmq

# Starte den Server
CMD ["./server"]
(base) root@DESKTOP-OL5SHQL:~/latenz-test/helloworld#