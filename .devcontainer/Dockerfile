FROM debian:bookworm

# Users and passwords
ARG BUILD_USER=build-user
ARG UID=1000
ARG GID=1000
ARG BUILD_USER_PWD=pwd

# Generate and set locale
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y locales \
    && sed -i -e 's/# en_US.UTF-8 UTF-8/en_US.UTF-8 UTF-8/' /etc/locale.gen \
    && dpkg-reconfigure --frontend=noninteractive locales \
    && rm -rf /var/lib/apt/lists/*

RUN locale-gen --purge en_US.UTF-8
RUN update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
ENV LC_ALL en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US.UTF-8

RUN apt-get update && apt-get install -y \
	sudo \
	git \
    vim \
    g++ \
    gdb \
    cmake \
    clang-format \
    ninja-build \
    pkg-config \
    zsh \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Repo dependencies
RUN apt-get update && apt-get install -y \
    libcapnp-dev \
    capnproto \
    python3-toml \
    && rm -rf /var/lib/apt/lists/*

# install Qt build common dependencies
RUN apt-get update && apt-get install -y \
    qtbase5-dev \
    qtdeclarative5-dev \
    libqt5websockets5-dev \
    libqt5webchannel5-dev \
    qml-module-qtquick-layouts \
    qml-module-qtquick-controls \
    qml-module-qtquick-controls2 \
    qml-module-qtquick-dialogs \
    qml-module-qtquick-shapes \
    qml-module-qt-websockets \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
    ripgrep \
    npm \
    nodejs \
    unzip \
    python3-venv \
    && rm -rf /var/lib/apt/lists/*

# Create group and user
RUN groupadd --gid=$GID $BUILD_USER
RUN useradd --create-home --shell /bin/bash $BUILD_USER --uid=$UID -g $BUILD_USER
RUN echo "$BUILD_USER:$BUILD_USER_PWD" | chpasswd
RUN usermod -aG sudo $BUILD_USER

USER $BUILD_USER
RUN wget -qO- https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh | zsh || true
RUN sed -i 's/ZSH_THEME="robbyrussell"/ZSH_THEME="gentoo"/g' ~/.zshrc

RUN echo 'alias vim=/home/build-user/nvim-linux64/bin/nvim' >> /home/build-user/.zshrc
RUN echo 'alias vim=/home/build-user/nvim-linux64/bin/nvim' >> /home/build-user/.bashrc

WORKDIR /home/build-user/capnzero
