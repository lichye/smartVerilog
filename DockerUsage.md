# Usage of Docker

## Install of Docker
Read https://docs.docker.com/engine/install/

## Images
*Images* is a copy of lib and sys

Using command line *pull* we can get images from docker hub.

    docker pull magna2024/smart

Then we can use *images* command line to see all the iamges we have.

    docker images

## Container
*Container* is a running model of *Images*. Each time when we run an images, we should and have to run in a *Container*

Below command line:
    docker run -it --rm smart
-it means run one container of *smart*
--rm means after exit of this container, delete the container.

## After getting into docker
Then it will be virtual environment of ubuntu for my image, just use the same command line in linux to work in the docker.

## Move files outside docker

    