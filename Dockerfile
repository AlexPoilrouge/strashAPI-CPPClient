FROM archlinux:latest

COPY ./dock/config/mirrorlist /etc/pacman.d/mirrorlist

RUN pacman-db-upgrade && \
    pacman -Syu --noconfirm --needed \
        base-devel wget git \
        gcc cmake gdb\
        nano \
        boost boost-libs \
        gtest \
        nlohmann-json

COPY ./dock/packages/* /var/aur/
WORKDIR /var/aur/
RUN ./getAURPkg.sh cpp-jwt

COPY . /strash_api_client
WORKDIR /strash_api_client/build

ARG STRASH=false
ARG COMPOSE_TEST=false
ARG CPP_DEBUG=false

RUN rm -rf ./* && sh -c "cmake .. $( ${STRASH} && echo '-DTEST_REALSTRASH=ON' ) $( ${COMPOSE_TEST} && echo '-DTEST_COMPOSE=ON' ) $( ${CPP_DEBUG} && echo '-DCMAKE_BUILD_TYPE=Debug' )" && make

RUN echo "strash: ${STRASH} $( ${STRASH} && echo '-DTEST_REALSTRASH=ON' ); debug: ${CPP_DEBUG} $( ${CPP_DEBUG} && echo '-DCMAKE_BUILD_TYPE=Debug' ); Compose: ${COMPOSE_TEST} $( ${COMPOSE_TEST} && echo '-DTEST_COMPOSE=ON' )"

COPY tests/data/private.key /app/keys/private.key

CMD [ "make", "run_tests" ]
