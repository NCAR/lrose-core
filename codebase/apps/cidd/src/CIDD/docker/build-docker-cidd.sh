#!/usr/bin/env bash

#
# script to build cidd Docker image for cidd 1.18
#
# update commit, version and release as needed
#

GIT_COMMIT=b343e777c
CIDD_VERSION=1.18
DOCKER_CIDD_RELEASE=0
DOCKER_REPOSITORY=ncareol/cidd

DOCKER_IMAGE=$DOCKER_REPOSITORY:$CIDD_VERSION-$DOCKER_CIDD_RELEASE

docker rmi $DOCKER_IMAGE $DOCKER_REPOSITORY:$CIDD_VERSION

git archive  -o lrose-cidd-$CIDD_VERSION.tgz $GIT_COMMIT

docker build --file docker/1.18.Dockerfile --tag $DOCKER_IMAGE .

#
# tag this release as latest of its version
#

docker tag $DOCKER_IMAGE $DOCKER_REPOSITORY:$CIDD_VERSION

echo
echo "  Docker image built and tags created, but they have not been pushed to Docker Hub yet"

echo "  To push image and its tags to Docker Hub, run:"
echo
echo "  docker push $DOCKER_IMAGE"
echo "  docker push $DOCKER_REPOSITORY:$CIDD_VERSION"
