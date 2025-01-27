#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

struct interface_info
{
    char name[IFNAMSIZ];         // Nom de l'interface
    char ipv4[INET_ADDRSTRLEN];  // Adresse IPv4
    char ipv6[INET6_ADDRSTRLEN]; // Adresse IPv6
};

// récupèrer les informations
void collect_interface_info(struct ifaddrs *ifaddr, struct interface_info *interfaces, int *count)
{
    struct ifaddrs *ifa;

    // on parcours toutes ls interfaces
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue; // Ignorer les interfaces sans adresse

        // On vérifie si cette interface est déjà ajoutée à la liste
        int found = -1;
        for (int i = 0; i < *count; i++)
        {
            if (strcmp(interfaces[i].name, ifa->ifa_name) == 0)
            {
                found = i;
                break;
            }
        }

        // si l'interface n'est pas trouvée, on ajoute une nouvelle interface avec des valeurs par défaut
        if (found == -1)
        {
            found = (*count)++;
            strncpy(interfaces[found].name, ifa->ifa_name, IFNAMSIZ - 1);
            interfaces[found].name[IFNAMSIZ - 1] = '\0';
            interfaces[found].ipv4[0] = '\0';
            interfaces[found].ipv6[0] = '\0';
        }

        // IPv4
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &(sa->sin_addr), interfaces[found].ipv4, sizeof(interfaces[found].ipv4));
        }
        // IPv6
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        {
            struct sockaddr_in6 *sa = (struct sockaddr_in6 *)ifa->ifa_addr;
            inet_ntop(AF_INET6, &(sa->sin6_addr), interfaces[found].ipv6, sizeof(interfaces[found].ipv6));
        }
    }
}

// afficher les informations de(s) interface(s)
void print_interface_info(struct interface_info *interfaces, int count)
{
    printf("Interface            IPv4                 IPv6\n");
    printf("-------------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++)
    {
        printf("%-20s %-20s %-40s\n",
               interfaces[i].name,
               interfaces[i].ipv4[0] ? interfaces[i].ipv4 : "N/A",
               interfaces[i].ipv6[0] ? interfaces[i].ipv6 : "N/A");
    }
}

// Fonction qui permet d'afficher les informations d'un interface (ifshow -i <nom_interface>)
void display_interface_informations(const char *interface_name)
{
    struct ifaddrs *ifaddr;
    struct interface_info interfaces[100];
    int count = 0;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    // Collecter les informations sur toutes les interfaces
    collect_interface_info(ifaddr, interfaces, &count);

    freeifaddrs(ifaddr);

    for (int i = 0; i < count; i++)
    {
        if (strcmp(interfaces[i].name, interface_name) == 0)
        {
            // On ne conserve que l'interface interface_name en l'ajoutant dans une struct
            struct interface_info single_interface = interfaces[i];
            print_interface_info(&single_interface, 1);
        }
    }
}

// Fonction qui permet de lister toutes les interfaces (ifshow -a)
void list_interfaces()
{
    struct ifaddrs *ifaddr;
    struct interface_info interfaces[100];
    int count = 0;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    collect_interface_info(ifaddr, interfaces, &count);

    freeifaddrs(ifaddr);

    print_interface_info(interfaces, count);
}

int main(int argc, char *argv[])
{
    if (argc == 3 && strcmp(argv[1], "-i") == 0)
    {
        // l'utilisateur souhaite afficher la liste des préfixes d’adresses IPv4 et IPv6 pour une interface donnée
        display_interface_informations(argv[2]);
    }
    else if (argc == 2 && strcmp(argv[1], "-a") == 0)
    {
        // l'utilisateur souhaite afficher la liste de toutes les interfaces réseaux, et leurs préfixes d'adresses IPv4 et IPv6
        list_interfaces();
    }
    else
    {
        // si l'utilisateur ne mets pas d'arguments ou des arguments incorrectes
        printf("Arguments incorectes ! Aide :\n");
        printf("ifshow -i <nom_interface>  : Affiche les prefixes d'adresses IPv4/IPv6 de <nom_interface>\n");
        printf("ifshow -a                  : Affiche toutes les interfaces avec leurs adresses IPv4/IPv6\n");
    }

    return 0;
}
