FROM codehz/node:latest AS builder

WORKDIR /build/stonejs2
ADD . /build/stonejs2
RUN npm i --unsafe-perm && npm prune
RUN /packager.sh -d /usr/bin/node /build

FROM scratch

COPY --from=builder /build /
