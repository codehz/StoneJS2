FROM codehz/node:latest AS builder

WORKDIR /build/stonejs2
ADD . /build/stonejs2
RUN npm i --unsafe-perm
RUN /packager.sh /usr/bin/node /build

FROM scratch

COPY --from=builder /build /
