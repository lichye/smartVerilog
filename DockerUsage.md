# Usage of Docker

## Install of Docker
Read https://docs.docker.com/engine/install/

## Images
*Images* is a copy of lib and sys.

Using command line *pull* we can get images from docker hub.

    docker pull magna2024/smart

Then we can use *images* command line to see all the iamges we have.

    docker images

We can remove images with *rmi* command line. [images_id] can be get in checking all the images command.
    
    docker rmi [images_id]

## Container
*Container* is a running model of *Images*. Each time when we run an images, we should and have to run in a *Container*

Below command line:

    docker run -it --rm smart

-it means create one container of *smart*
--rm means after exit of this container, delete the container.

To check the remaining containers:

    docker ps -a

ps means check the containers
-a means check all the containers. Without this, we cannot find the stop containers

To remove the remaining containers:

    docker rm [contianer_id]

If one container is running, we need to stop it firstly.

    docker stop [contianer_id]

## After getting into docker
Then it will be virtual environment of ubuntu for my image, just use the same command line in linux to work in the docker.

