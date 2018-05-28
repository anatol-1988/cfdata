FROM gcc:7

RUN curl https://cmake.org/files/v3.11/cmake-3.11.2-Linux-x86_64.sh -o /tmp/curl-install.sh \
      && chmod u+x /tmp/curl-install.sh \
      && mkdir /usr/bin/cmake \
      && /tmp/curl-install.sh --skip-license --prefix=/usr/bin/cmake \
      && rm /tmp/curl-install.sh

ENV PATH="/usr/bin/cmake/bin:${PATH}"

COPY . /usr/src/myapp
WORKDIR /usr/src/myapp

RUN cmake -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ --build .

RUN make cfdata

CMD ["./cfdata"]
