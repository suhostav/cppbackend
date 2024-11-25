FROM gcc:11.3 as build
# Выполнить установку зависимостей внутри контейнера.
RUN apt update && \
    apt install -y \
      python3-pip \
      cmake

#RUN apt install python3
RUN pip3 install conan==1.*
   
#copy conanfile.txt and run conan install ...
COPY conanfile.txt /app/
RUN mkdir /app/build && cd /app/build && \
    conan install .. --build=missing -s compiler.libcxx=libstdc++11 -s build_type=Release

# Скопировать файлы проекта внутрь контейнера
COPY ./src /app/src
COPY ./tests /app/tests
COPY CMakeLists.txt /app/ 

RUN cd /app/build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . 

#-------- run container ---------------------
FROM ubuntu:22.04 as run

RUN groupadd -r www && useradd -r -g www www
RUN ["mkdir", "/volume"]
USER www

COPY --from=build /app/build/game_server /app/
COPY --from=build /app/build/game_server_tests /app/
COPY ./data /app/data
COPY ./static /app/static

ENTRYPOINT ["/app/game_server", "-с /app/data/config.json", "-w /app/static"]
