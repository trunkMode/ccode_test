int get_ifaddr(const char *match, ifaddr_t **ifaddr)
{
    int i;
    wl_interfaces_t intfs;
    struct sockaddr addr;
    ifaddr_t *ifaddr_tmp, *p;

    memset(&intfs, 0x00, sizeof(intfs));
    common_get_desired_devices(match, &intfs, NULL);

    for (i = 0; i < intfs->intf_cnt; i++) {
        if ((ifaddr_tmp = malloc(sizeof(ifaddr_t))) {
            memset(ifaddr_tmp, 0x00, sizeof(ifaddr_t));
            memcpy(ifaddr_tmp->ifname, intfs->intf[i].ifname, sizeof(ifaddr_tmp->ifname));

            get_interface_ipv4(ifaddr_tmp->ifname, &ifaddr_tmp->if_addr, SIOCGIFADDR);
            get_interface_ipv4(ifaddr_tmp->ifname, &ifaddr_tmp->if_netmask, SIOCGIFNETMASK);

            if (*ifaddr == NULL) {
                *ifaddr = ifaddr_tmp;
                p = *ifaddr;
            } else {
                p->next = ifaddr_tmp;
                p = p->next;
            }
        }
    }
}
