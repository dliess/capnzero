#!/bin/bash

function print_usage
{
	echo "usage: $0 <repo dir>"
}

if [[ $# != 1 ]]
then
	print_usage
	exit 1
fi

REPO_DIR=$(readlink -f $1)

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
source "$DIR/common.sh"

docker run \
         --network=host \
         --mount type=bind,source=$HOME/.ssh,target=/home/build-user/.ssh \
         --mount type=bind,source=$HOME/.gitconfig,target=/home/build-user/.gitconfig \
         --mount type=bind,source=$HOME/.config/nvim,target=/home/$BUILD_USER/.config/nvim \
         --mount type=bind,source=$HOME/.config/github-copilot,target=/home/$BUILD_USER/.config/github-copilot \ 
         --mount type=bind,source=$HOME/nvim-linux64,target=/home/$BUILD_USER/nvim-linux64 \
         --mount type=bind,source=$REPO_DIR,target=/home/$BUILD_USER/capnzero \
           -ti $DOCKER_IMAGE_TAG:latest /bin/zsh 

## logging in to a running container:
## get id with: docker ps
## docker exec -it <id> /bin/bash
