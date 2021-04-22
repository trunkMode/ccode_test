#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
int gen_suricata_afp_mapping()
{
#define SURICATA_AFP_MAPPING_YAML   "./af-packet-mapping.yaml"
    int unit, bss;
    FILE *fp = NULL;

    if (!(fp = fopen(SURICATA_AFP_MAPPING_YAML, "w"))) {
    	printf("Failed to open %s\n", SURICATA_AFP_MAPPING_YAML);
//        CRITICAL_DEBUG((__FUNCTION__, __LINE__, "FATAL ERR: Cannot create af-packet mapping file!\n"));
        return -1;
    }
    fprintf(fp, "%YAML 1.1\n---\n");
    fprintf(fp, "af-packet:\n");
    for (unit =  0 ; unit < 2 ; unit++) {
            for (bss = 0; bss < 7; bss++) {
		 		{
                    fprintf(fp, "  - interface: ath%d%d", unit, bss);
                    fprintf(fp, "    threads: auto\n");
                    fprintf(fp, "    cluster-id: 98\n");
                    fprintf(fp, "    defrag: yes\n");
                    fprintf(fp, "    cluster-type: cluster_flow\n");
                    fprintf(fp, "    copy-mode: ips\n");
                    fprintf(fp, "    copy-iface: tapSec_ath%d%d\n", unit, bss);
                    fprintf(fp, "    buffer-size: 32768\n");
                    fprintf(fp, "\n");
                    fprintf(fp, "  - interface: tapSec_ath%d%d", unit, bss);
                    fprintf(fp, "    threads: auto\n");
                    fprintf(fp, "    cluster-id: 98\n");
                    fprintf(fp, "    defrag: yes\n");
                    fprintf(fp, "    cluster-type: cluster_flow\n");
                    fprintf(fp, "    copy-mode: ips\n");
                    fprintf(fp, "    copy-iface: ath%d%d\n", unit, bss);
                    fprintf(fp, "    buffer-size: 32768\n\n");
                    fprintf(fp, "\n");                    
                }
            }
    }

    fclose(fp);
    return 0;
}

int main()
{
	gen_suricata_afp_mapping();
	return 0;
}
