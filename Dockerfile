FROM codehz/node:latest

WORKDIR /build/stonejs2
ADD . /build/stonejs2
RUN npm i --unsafe-perm
