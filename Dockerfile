FROM rferraro/cxx-clang:5.0

RUN apt-get update && apt-get -y install libcurl4-openssl-dev

COPY . /usr/src/myapp
WORKDIR /usr/src/myapp

RUN cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ --build .

RUN make cfdata

CMD ["./cfdata"]
