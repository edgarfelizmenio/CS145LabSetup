# CS 145 TL2 and TL3 setup scripts

1. Getting all IP addresses
    ```
    nmap -n -sn 192.168.145.21-100 -oG - | awk '/Up$/{print$2}' > cs145tl.ini
    ```
2. Add all computers to known_hosts and copy their ssh keys
    ```
    ansible-playbook -i cs145tl.ini add-ssh-keys.yaml
    ```
3. Execute setup script
    ```
    ansible-playbook -i cs145tl.ini -k -K -u faculty --become bootstrap.yaml
    ```