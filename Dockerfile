FROM python:3.11-slim

# System dependencies
RUN apt-get update && apt-get install -y \
    git \
    curl \
    build-essential \
    udev \
    && rm -rf /var/lib/apt/lists/*

# PlatformIO installieren
RUN pip install --no-cache-dir platformio

# User für bessere Permissions
ARG USER_UID=1000
ARG USER_GID=1000
RUN groupadd -g ${USER_GID} devuser && \
    useradd -m -u ${USER_UID} -g ${USER_GID} -s /bin/bash devuser

# Workspace
WORKDIR /workspace
RUN chown devuser:devuser /workspace

USER devuser

# PlatformIO Cache Verzeichnis
RUN mkdir -p /home/devuser/.platformio

ENTRYPOINT ["platformio"]
CMD ["--help"]
