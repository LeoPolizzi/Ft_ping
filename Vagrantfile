Vagrant.configure("2") do |config|
  # Use the Bento Debian 7.11 box
  config.vm.box = "bento/debian-7.11"

  # Optional: set VM name and memory
  config.vm.provider "virtualbox" do |vb|
    vb.name = "debian7-vm"
    vb.memory = "1024"
    vb.cpus = 1
  end

  # Configure SSH
  config.ssh.username = "vagrant"
  config.ssh.password = "vagrant"
  config.ssh.insert_key = true

  # Provisioning: install inetutils (ping) version 2.0.0 from GNU
  config.vm.provision "shell", inline: <<-SHELL
    echo "Updating APT to archive.debian.org..."
    sudo sed -i 's|http://.*debian.org|http://archive.debian.org/debian|g' /etc/apt/sources.list
    sudo apt-get update -o Acquire::Check-Valid-Until=false -y

    echo "Installing build dependencies..."
    sudo apt-get install -y build-essential wget

    echo "Downloading GNU inetutils 2.0..."
    wget https://ftp.gnu.org/gnu/inetutils/inetutils-2.0.tar.gz
    tar xzf inetutils-2.0.tar.gz
    cd inetutils-2.0
    ./configure
    make
    sudo make install

    echo "Cleaning up..."
    cd ..
    rm -rf inetutils-2.0*
  SHELL
end
