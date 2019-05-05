#===================================================
# build lrose in suse container

ARG LROSE_PKG=cidd
ARG RELEASE_DATE=latest

RUN \
    echo "Building lrose in suse container"; \
    echo "  package is: ${LROSE_PKG}" 

# run the checkout and build script

RUN \
    /scripts/checkout_and_build_auto.py \
    --package lrose-${LROSE_PKG} \
    --releaseDate ${RELEASE_DATE} \
    --prefix /usr/local/cidd \
    --buildDir /tmp/cidd_build \
    --logDir /tmp/cidd_build_logs

# clean up

RUN \
    /bin/rm -rf /tmp/cidd_build
