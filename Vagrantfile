Vagrant.configure("2") do |config|

  config.vm.box = "bento/debian-7.11"

  config.vm.provider "virtualbox" do |vb|
    vb.name = "debian7-vm"
    vb.memory = "1024"
    vb.cpus = 1
  end

  config.ssh.username = "vagrant"
  config.ssh.password = "vagrant"
  config.ssh.insert_key = true

  config.vm.provision "shell", inline: <<-SHELL
    BLUE='\\033[0;34m'
    YELLOW='\\033[1;33m'
    MAGENTA='\\033[0;35m'
    NC='\\033[0m'

    echo -e "${BLUE}Updating APT to archive.debian.org...${NC}"
    sudo sed -i 's|http://.*debian.org|http://archive.debian.org/debian|g' /etc/apt/sources.list
    sudo apt-get update -o Acquire::Check-Valid-Until=false -y

    echo -e "${YELLOW}Installing build dependencies...${NC}"
    sudo apt-get install -y build-essential wget

    echo -e "${MAGENTA}Downloading GNU inetutils 2.0...${NC}"
    wget https://ftp.gnu.org/gnu/inetutils/inetutils-2.0.tar.gz
    tar xzf inetutils-2.0.tar.gz
    cd inetutils-2.0
    ./configure
    make
    sudo make install

    echo -e "${BLUE}Cleaning up...${NC}"
    cd ..
    rm -rf inetutils-2.0*
  SHELL
end
