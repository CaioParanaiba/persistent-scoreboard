#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define COR_VERMELHA "\x1b[31m"
#define COR_VERDE    "\x1b[32m"
#define COR_AMARELA  "\x1b[33m"
#define COR_AZUL     "\x1b[34m"
#define RESETAR_COR  "\x1b[0m"
#define COR_CIANO    "\x1b[36m"

typedef struct
{
    char nickname[50];
    long long int point;         //pontuacao do competidor
    int solve;                  //numero de questoes resolvidas
    int penality;               //penalidade: seria o tempo gasto total para fazer a competicao
    float time;                 //tempo medio gasto nos problemas
    int ativo;                  //verficar exclusao ou nao
}competidor;

typedef struct{
    char nickname[50];
    int id;
}id_nome;

typedef struct
{
    long long int point;
    int id;
}id_point;


int p=-1;           //usado para saber o tamanho atual do vetor
int ativos=0;       //usado para saber se e possivel listar competidores ou nao


int BuscaBinN(id_nome *indice_nome,char *x,int l,int r) {
    //busca binaria por nome que retorna -1 para nao encontrado ou o indice se encontrado
    if(l==r) {
        if(strcmp(x,indice_nome[l].nickname)==0) return indice_nome[l].id;
        return -1;
    }

    int m = (l+r) >> 1;
    if(strcmp(x,indice_nome[m].nickname)>0) return BuscaBinN(indice_nome,x,m+1,r);
    return BuscaBinN(indice_nome,x,l,m);
}

int BuscaBinP(id_point *indice_point, long long int x, int l, int r) {
    //busca binaria por pontos que retorna -1 para nao encontrado ou o indice se encontrado
    if (l == r) {
        if (indice_point[l].point == x) return indice_point[l].id;
        return -1;
    }

    int m = (l + r) >> 1;
    if (x < indice_point[m].point) return BuscaBinP(indice_point, x, m + 1, r); 
    return BuscaBinP(indice_point, x, l, m);
}


void Inserir_nome(competidor novo,id_nome *indice_nome,int i){
    //funcao para inserir o indice de nome do novo ompetidor na posicao correta

    //facos as trocas sem usar variaveis auxiliares, encontro a posicao e so depois insiro o competidor novo
    while (i>0 && strcmp(indice_nome[i-1].nickname,novo.nickname)>0) {
        indice_nome[i] = indice_nome[i-1];
        i--;
    }

    indice_nome[i].id=p;
    strcpy(indice_nome[i].nickname,novo.nickname);
}

void Inserir_point(competidor novo,id_point *indice_point,int i){
    //funcao para inserir o indice de ponto do novo ompetidor na posicao correta

    //facos as trocas sem usar variaveis auxiliares, encontro a posicao e so depois insiro o competidor novo
    while (i>0 && indice_point[i-1].point < novo.point) {
        indice_point[i] = indice_point[i-1];
        i--;
    }

    indice_point[i].id=p;
    indice_point[i].point=novo.point;
}

void CreatePoint(competidor *novo,id_point *indice_point,int id,int times,int soma_penality){
    //funcao para calcular os pontos que o competidor fez com base no numero de solves que ele teve

    printf(COR_CIANO "\n  --- REGISTRO TEMPO GASTO ---\n" RESETAR_COR);
    for (int t=1;t<=times;t++) {
        int penality;
        printf("   > Tempo do " COR_AMARELA "Problema %d" RESETAR_COR " (segundos): ", t);
        scanf("%d",&penality);
        soma_penality += penality;
    }

    novo->penality = soma_penality;

    novo->time = (novo->penality/60.0) / novo->solve;               //convertanto segundos para minutos (já garantido que solve>0)
    
    /*
    pontuacao baseada no numero de solucoes de forma primordial!!
    com desempate por penalidade, e usando o metodo de desempate com os ids
    O medodo de desempate dos ids, faz com que os outros dois metodos nao interfiram nas ultimas casas da pontuacao
    permitindo que as pontuacoes sejam sempre unicas e EXCLUSIVAS, sendo possivel usa-la na busca
    */
    novo->point = (novo->solve * 100000000LL) - (novo->penality * 1000LL) - id;

    Inserir_point(*novo,indice_point,id);                         //incrementar um novo indice
}

void Addsolve(competidor *novo,id_point *indice_point,int id){
    //funcao que adiciona um numero a mais de problemas resolvidos
    int times;
    printf("Quantas questoes a mais foram feitas? ");
    scanf("%d",&times);
    novo->solve+=times;
    CreatePoint(novo,indice_point,id,times,novo->penality);
}

void Inserir(FILE *arq_bd,id_nome *indice_nome,id_point *indice_point){
    char ans='s';
    
    while(ans=='s' && p<1000) {
        system("clear");
        printf(COR_CIANO "\n--- CADASTRAR NOVO COMPETIDOR ---\n" RESETAR_COR);
        p++;
        ativos++;

        competidor novo;

        novo.ativo=1; // por padrao ativa os dados novos

        int namevalid=0;                                                        //verificar se o nome é valido

        while(!namevalid){
            printf("Escreva o nickname do competidor:\n> ");
            scanf("%s",novo.nickname);

            int achei=-1;                                                       //assumimos nao ter um clone

            if(p>0) achei = BuscaBinN(indice_nome,novo.nickname,0,p-1);        //buscamos em um vetor que ja existe

            //nao consigo verificar rapidamente se se o o competidor na posicao achei é valido ou nao, por estar salvo no arquivo binario
            if(achei!=-1){
                competidor fantasma;
                fseek(arq_bd,achei*sizeof(competidor),SEEK_SET);
                fread(&fantasma,sizeof(competidor),1,arq_bd);
                if(fantasma.ativo==0) achei=-1;
            }

            if(achei==-1) namevalid=1;                    //ou nao existe, oou se existe o nome foi excluido
            else printf(COR_VERMELHA "\nErro: Nome de usuario ja existente! Tente outro.\n\n" RESETAR_COR);
        }

        Inserir_nome(novo,indice_nome,p);

        printf("Quantos problemas foram feitos?\n> ");
        scanf("%d",&novo.solve);

        if(novo.solve>0) CreatePoint(&novo,indice_point,p,novo.solve,0);
        else{
            novo.point=-p;
            Inserir_point(novo,indice_point,p);
        }

        //colocar no arquivo o novo competidor
        fseek(arq_bd,p*sizeof(competidor),SEEK_SET);
        fwrite(&novo,sizeof(competidor),1,arq_bd);
        fflush(arq_bd);

        printf(COR_AMARELA "\nDeseja incluir mais algum usuario? (s/n)\n> " RESETAR_COR);
        scanf(" %c",&ans); //eliminar o buffer do \n do ultimo scanf dando espaco
    }
    printf("\n");
    system("Clear");

    if(p>=1000) printf(COR_VERMELHA "\nMemoria Insuficiente Para Inserir/Alterar Novos Dados\n"RESETAR_COR);        //aviso caso supere o limite do meu malloc
}

void List(FILE *arq_bd){

    printf("Lista Competidores:\n\n");

    printf(COR_CIANO "%-5s %-20s %-8s %-15s %-15s\n" RESETAR_COR, "ID", "Nickname", "Solves", "Pontuacao", "Tempo Medio");

    competidor atual;
    for(int i=0;i<=p;i++) {
        fseek(arq_bd,i*sizeof(competidor),SEEK_SET);                //VOU PARA A POSIÇÃO NECESSARIA
        fread(&atual,sizeof(competidor),1,arq_bd);
        //verificar que esta ativo, se sim informar valores
        if(atual.ativo==1){
            printf("%-5d %-20s %-8d %-15lld %-7.2f(min)\n", i, atual.nickname, atual.solve, atual.point, atual.time);
        }
    }

}

void ListN(FILE *arq_bd,id_nome *indice_nome){
    printf("Lista Competidores em Ordem Alfabetica:\n\n");

    printf(COR_CIANO "%-5s %-20s %-8s %-15s %-15s\n" RESETAR_COR, "ID", "Nickname", "Solves", "Pontuacao", "Tempo Medio");

    competidor atual;
    for (int i=0;i<=p;i++) {
        int id = indice_nome[i].id;
        fseek(arq_bd,id*sizeof(competidor),SEEK_SET);                //VOU PARA A POSIÇÃO NECESSARIA
        fread(&atual,sizeof(competidor),1,arq_bd);
        //verificar que esta ativo, se sim informar valores
        if (atual.ativo==1) { 
            printf("%-5d %-20s %-8d %-15lld %-7.2f(min)\n", id, atual.nickname, atual.solve, atual.point, atual.time);
        }
    }
}

void ListP(FILE *arq_bd,id_point *indice_point){
    printf("Lista Competidores em Ordem de Pontuacao:\n\n");

    printf(COR_CIANO "%-5s %-20s %-8s %-15s %-15s\n" RESETAR_COR, "ID", "Nickname", "Solves", "Pontuacao", "Tempo Medio");

    competidor atual;
    for (int i=0;i<=p;i++) {
        int id = indice_point[i].id;
        fseek(arq_bd,id*sizeof(competidor),SEEK_SET);                //VOU PARA A POSIÇÃO NECESSARIA
        fread(&atual,sizeof(competidor),1,arq_bd);
        //verificar que esta ativo, se sim informar valores
        if (atual.ativo==1) {
            printf("%-5d %-20s %-8d %-15lld %-7.2f(min)\n", id, atual.nickname, atual.solve, atual.point, atual.time);
        }
    }
}

void Listar(FILE *arq_bd,id_nome *indice_nome,id_point *indice_point){
    //funcao para escolher qual o modo de listagem

    int opc;

    system("clear"); // Limpa a tela antes de mostrar o menu de listagem
    printf(COR_CIANO "\n--- MENU DE LISTAGEM ---\n" RESETAR_COR);
    printf("Voce deseja listar por qual categoria?\n");
    printf(COR_AZUL "[1]" RESETAR_COR " Padrao (ID)\n");
    printf(COR_AZUL "[2]" RESETAR_COR " Ordem Alfabetica (Nome)\n");
    printf(COR_AZUL "[3]" RESETAR_COR " Ranking (Pontos)\n");
    printf("Opcao: ");
    
    scanf("%d", &opc);
    
    system("clear"); // Limpa o menu para a tabela brilhar sozinha na tela
    
    if(opc == 1) List(arq_bd);
    else if(opc == 2) ListN(arq_bd, indice_nome);
    else if(opc == 3) ListP(arq_bd, indice_point);
}

void Excluir(FILE *arq_bd,int id){
    competidor atual;

    fseek(arq_bd,id*sizeof(competidor),SEEK_SET);
    fread(&atual,sizeof(competidor),1,arq_bd);
    atual.ativo=0;                     //exclui
    ativos--;                           //diminui os ativos

    fseek(arq_bd, id*sizeof(competidor), SEEK_SET); // O fread andou 1 pra frente, precisamos voltar!
    fwrite(&atual, sizeof(competidor), 1, arq_bd);  // Sobrescrevemos o cara morto no HD
    fflush(arq_bd);                                 // Forçamos o salvamento
}

void Alterar(FILE *arq_bd, id_nome *indice_nome, id_point *indice_point, int id) {
    //Lemos o competidor original do HD
    competidor velho;
    fseek(arq_bd, id * sizeof(competidor), SEEK_SET);
    fread(&velho, sizeof(competidor), 1, arq_bd);

    char ans_nome = 'n';
    char ans_pontos = 'n';

    printf("\nDeseja alterar o nickname? (s/n)\n> ");
    scanf(" %c", &ans_nome);

    printf("Os pontos atuais sao: " COR_VERDE "%lld" RESETAR_COR "\nDeseja alterar os pontos? (s/n)\n> ", velho.point);
    scanf(" %c", &ans_pontos);

    // Se ele não quer mudar nada fdc, voltamos
    if (ans_nome == 'n' && ans_pontos == 'n') {
        printf(COR_AMARELA "Nenhuma alteracao realizada.\n" RESETAR_COR);
        return; 
    }

    p++; 
    competidor novo = velho; // Copiamos

    // --- TRATANDO NOME ---
    if (ans_nome == 's') {
        int namevalid = 0;
        while (!namevalid) {
            printf("Escreva o novo nickname do competidor:\n> ");
            scanf("%24s", novo.nickname); // %24s para previnir o Stack Smashing!

            int achei = -1;
            if (p > 0) achei = BuscaBinN(indice_nome, novo.nickname, 0, p - 1);

            // Verificação do Fantasma no disco
            if (achei != -1) {
                competidor fantasma;
                fseek(arq_bd, achei * sizeof(competidor), SEEK_SET);
                fread(&fantasma, sizeof(competidor), 1, arq_bd);
                if (fantasma.ativo == 0) achei = -1; 
            }

            if (achei == -1) namevalid = 1;
            else printf(COR_VERMELHA "\nErro: Nome de usuario ja existente! Tente outro.\n\n" RESETAR_COR);
        }
    }
    // Grava o nome (novo ou velho) no índice com o ID novo (p)
    Inserir_nome(novo, indice_nome, p);


    // --- TRATANDO PONTOS ---
    if (ans_pontos == 's') {
        int opc;
        printf(COR_AZUL "\n[1]" RESETAR_COR " Refazer Pontos do zero\n");
        printf(COR_AZUL "[2]" RESETAR_COR " Acrescentar novas Solucoes\n");
        printf("Opcao: ");
        scanf("%d", &opc);

        if (opc == 1) {
            printf("Quantos problemas foram feitos?\n> ");
            scanf("%d", &novo.solve);

            if (novo.solve > 0) {
                CreatePoint(&novo, indice_point, p, novo.solve, 0);
            } else {
                novo.point = -p;
                Inserir_point(novo, indice_point, p);
            }
        } else {
            Addsolve(&novo, indice_point, p);
        }
    } else {
        // MUITO IMPORTANTE: Se o usuário mudou SÓ o nome, o ID dele mudou de 'id' para 'p'.
        // Temos que recalcular os pontos para o desempate pegar o ID novo!
        novo.point = (novo.solve * 100000000LL) - (novo.penality * 1000LL) - p;
        Inserir_point(novo, indice_point, p);
    }

    // --- SALVANDO TUDO ---
    fseek(arq_bd, p * sizeof(competidor), SEEK_SET);
    fwrite(&novo, sizeof(competidor), 1, arq_bd);

    // 2. Transforma o antigo em fantasma no disco!
    Excluir(arq_bd, id);
    
    fflush(arq_bd); 
}

int lower_bound(id_point *indice_point,long long int limite_minimo,int l,int r){
    //busca binaria queremos peli menos o final
    if (l == r) {
        return l;
    }

    int m = (l + r) >> 1;
    if (limite_minimo < indice_point[m].point) return lower_bound(indice_point, limite_minimo, m + 1, r); 
    return lower_bound(indice_point, limite_minimo, l, m);
}

int upper_bound(id_point *indice_point,long long int limite_maximo,int l,int r){
    //busca binaria nao queremos o valor exato, queremos pelo menos o inicio
    if (l == r) {
        return l;
    }

    int m = (l + r) >> 1;
    if (limite_maximo < indice_point[m].point) return upper_bound(indice_point, limite_maximo, m + 1, r); 
    return upper_bound(indice_point, limite_maximo, l, m);
}

void Buscar(FILE *arq_bd,id_nome *indice_nome,id_point *indice_point){
    //funcao usadam apenas como menu das buscas e apresentacao de resultados
    competidor atual;
    int opc=-1;
    while(opc){

        system("clear");

        printf(COR_CIANO "\n--- MENU DE BUSCA ---\n" RESETAR_COR);
        printf("Voce deseja fazer uma busca por qual categoria?\n");
        printf(COR_AZUL "[1]" RESETAR_COR " ID\n");
        printf(COR_AZUL "[2]" RESETAR_COR " Nome\n");
        printf(COR_AZUL "[3]" RESETAR_COR " Pontos\n");
        printf(COR_AZUL "[4]" RESETAR_COR " Busca em Range de Solucoes\n");
        printf(COR_VERMELHA "[0]" RESETAR_COR " Voltar ao Menu Principal\n");
        printf("Opcao: ");
        
        scanf("%d", &opc);
        int id=-1;
        
        if(opc==1) {
            int x;
            printf("Qual o ID?\n> ");
            scanf("%d",&x);

            fseek(arq_bd,x*sizeof(competidor),SEEK_SET);
            fread(&atual,sizeof(competidor),1,arq_bd);
            
            //busca binaria desnecessario, e a porra do indice so pegar krl
            if (x >= 0 && x <= p && atual.ativo == 1) {
                id = x;
            } else {
                id = -1;
            }
        }
        else if(opc==2) {
            char x[25];
            printf("Qual o nickname?\n> ");
            scanf("%s",x);
            id = BuscaBinN(indice_nome,x,0,p);
        }
        else if(opc==3) {
            int x;
            printf("Qual a Pontuacao?\n> ");
            scanf("%lld",&x);
            id = BuscaBinP(indice_point,x,0,p);
        }
        else if(opc==4){
            int min_solves, max_solves;
            
            printf("Digite o range de problemas resolvidos\n");
            printf("Digite o numero MINIMO de problemas:\n> ");
            scanf("%d", &min_solves);
            while(min_solves<0){
                printf(COR_AMARELA"ERRO: Digite o numero MINIMO POSITIVO de problemas:\n> "RESETAR_COR);
                scanf("%d", &min_solves);
            }
            
            printf("Digite o numero MAXIMO de problemas:\n> ");
            scanf("%d",&max_solves);
            while(max_solves<0){
                printf(COR_AMARELA"ERRO: Digite o numero MAXIMO POSITIVO de problemas:\n> "RESETAR_COR);
                scanf("%d", &max_solves);
            }
            
            // Caso escreva invertido ele troca, usuario burro
            if (min_solves > max_solves) {
                int aux = min_solves;
                min_solves = max_solves;
                max_solves = aux;
            }

            //AQUI ENTRA a magia do meu desempate
            //os solves estao na casa dos 100milhoes na pontuacao, ou seja podemos buscar a partir deles
            long long int n_max_solves = (long long int)(max_solves) * 100000000LL;
            long long int n_min_solves = (long long int)(min_solves-1) * 100000000LL;

            int inicio = upper_bound(indice_point, n_max_solves, 0, p + 1);
            int fim = lower_bound(indice_point, n_min_solves, 0, p + 1);

            if (inicio == fim || inicio > p) {
                printf(COR_VERMELHA"\nNenhum competidor encontado nesse intervalo!\n"RESETAR_COR);
            } else {
                system("clear");
                printf(COR_VERDE"\nCompetidores encontrados (%d a %d problemas):\n\n"RESETAR_COR, min_solves, max_solves);
                printf(COR_CIANO "%-5s %-20s %-8s %-15s %-15s\n" RESETAR_COR, "ID", "Nickname", "Solves", "Pontuacao", "Tempo Medio");
                
                //rodamos o intervalo
                for (int k = inicio; k < fim; k++) {
                    int id = indice_point[k].id;
                    fseek(arq_bd,id*sizeof(competidor),SEEK_SET);
                    fread(&atual,sizeof(competidor),1,arq_bd);
                    if (atual.ativo == 1) { // Só os não-fantasmas
                        printf("%-5d %-20s %-8d %-15lld %-7.2f(min)\n", id, atual.nickname, atual.solve, atual.point, atual.time);
                    }
                }
            }
            char ans='n';
            printf("\nDeseja fazer uma nova busca? (s/n)\n> ");
            scanf(" %c", &ans);
            if(ans == 'n') opc = 0;
        }

        if(opc>0 && opc<4) {
            fseek(arq_bd,id*sizeof(competidor),SEEK_SET);
            fread(&atual,sizeof(competidor),1,arq_bd);
            if(id>=0 && atual.ativo==1){           //caso eu tenha alterado um valor e eu encontre justamente ele, sendo que ele foi excluido eu consigo ver isso
                system("clear");
                printf(COR_VERDE"Competidor Encontrado:\n\n"RESETAR_COR);

                printf(COR_CIANO "%-5s %-20s %-8s %-15s %-15s\n" RESETAR_COR, "ID", "Nickname", "Solves", "Pontuacao", "Tempo Medio");
                printf("%-5d %-20s %-8d %-15lld %-7.2f(min)\n\n", id, atual.nickname, atual.solve, atual.point, atual.time);

                int opc1;
                printf(COR_AMARELA "\nO que deseja fazer com este competidor?\n" RESETAR_COR);
                printf(COR_VERMELHA "[1]" RESETAR_COR " Excluir\n");
                printf(COR_AZUL "[2]" RESETAR_COR " Alterar Dados\n");
                printf(COR_VERDE "[3]" RESETAR_COR " Manter (Nao fazer nada)\n");
                printf("Opcao: ");
                
                scanf("%d", &opc1);
                if(opc1==1) Excluir(arq_bd,id);
                if(opc1==2 && p<1000) Alterar(arq_bd,indice_nome,indice_point,id);
                else if(opc1==2 && p>=1000) printf(COR_VERMELHA"\nMemoria Insuficiente Para Inserir/Alterar Novos Dados\n"RESETAR_COR);    //aviso caso supere o limite do meu malloc
            }
            else printf(COR_VERMELHA"Competidor nao encontrado!\n\n"RESETAR_COR);

            char ans='n';
            printf("Deseja fazer uma nova busca? (s/n) ");
            scanf(" %c",&ans);
            if(ans=='n') opc = 0;
        }
    }
}

int main(){
    //pensando na regra futura de desempate 1000 é o tamanho maximo, pois de tiver 1000 iguais ainda nao terei o problema de pontos
    //competidor* bd = (competidor *)malloc(sizeof(competidor)*1000);       #foi removido o bd principal, por nao trabalharmos na ram com ele
    id_nome* indice_nome = (id_nome*)malloc(sizeof(id_nome)*1000);
    id_point* indice_point = (id_point*)malloc(sizeof(id_point)*1000);

    if(indice_nome==NULL || indice_point==NULL) {
        printf("Memoria Cheia\n");
        return 1;
    }

    FILE *arq_bd = fopen("bd_competidores.bin","rb+");
    if(arq_bd==NULL){
        arq_bd = fopen("bd_competidores.bin","wb+");
    }

    FILE *arq_idx_nome = fopen("idx_nome.bin","rb+");
    if(arq_idx_nome==NULL){
        arq_idx_nome = fopen("idx_nome.bin","wb+");
    }

    FILE *arq_idx_point = fopen("idx_point.bin","rb+");
    if(arq_idx_point==NULL){
        arq_idx_point = fopen("idx_point.bin","wb+");
    }

    int lidos_nome = fread(indice_nome,sizeof(id_nome),1000,arq_idx_nome);
    int lidos_point =  fread(indice_point,sizeof(id_point),1000,arq_idx_point);

    // ja podemos fechar o nossos indices, mas ferei no final, para melhor organizacao

    //atualizar as minhas variaveis globais:
    if(lidos_nome>0){
        p = lidos_nome -1;
        ativos = 0; // Zeramos para contar de verdade
        
        // Damos uma corrida rápida no banco para contar quem está vivo
        competidor temp;
        for(int i = 0; i <= p; i++){
            fseek(arq_bd, i*sizeof(competidor), SEEK_SET);
            fread(&temp, sizeof(competidor), 1, arq_bd);
            if(temp.ativo == 1){
                ativos++;
            }
        }
    }

    system("clear");

    printf("Bem vindo ao Pseudo Banco de Dados\n");

    //Menu dos servicos
    int opc = -1;
    while (opc){
        printf(COR_CIANO "\n============================================\n" RESETAR_COR);
        printf(COR_CIANO "    BANCO DE DADOS - MARATONA DE CODIGO     \n" RESETAR_COR);
        printf(COR_CIANO "============================================\n" RESETAR_COR);

        printf("\nEscolha uma das opcoes abaixo:\n");
        printf(COR_VERDE "[1]" RESETAR_COR " Inserir novos dados\n");
        if(ativos > 0) {
            printf(COR_AZUL "[2]" RESETAR_COR " Listar os dados\n");
            printf(COR_AMARELA "[3]" RESETAR_COR " Buscar/Excluir/Alterar valores\n");
        }
        printf(COR_VERMELHA "[0]" RESETAR_COR " Sair\n");
        printf("Opcao: ");
        scanf("%d",&opc);

        if(opc==1){
            if(p<1000) Inserir(arq_bd,indice_nome,indice_point);
            else printf(COR_VERMELHA"\nMemoria Insuficiente Para Inserir/Alterar Novos Dados\n"RESETAR_COR);           //aviso caso supere o limite do meu malloc
        }
        if (ativos>0){
            if(opc==2) Listar(arq_bd,indice_nome,indice_point);
            else if(opc==3) Buscar(arq_bd,indice_nome,indice_point);
        }
    }

    system("clear");
    printf("\nObrigado por ter usado esse Pseudo Banco de dados\n\nSalvarei suas informacoes para quando voce voltar\n");


    //precisamos fechar todos os arquivos
    fclose(arq_bd);
    fclose(arq_idx_nome);
    fclose(arq_idx_point);

    //reescrever os arquivos binarios do indice
    arq_idx_nome = fopen("idx_nome.bin","wb");
    arq_idx_point = fopen("idx_point.bin","wb");

    if(arq_idx_nome!=NULL){
        fwrite(indice_nome,sizeof(id_nome),p+1,arq_idx_nome);
        fclose(arq_idx_nome);
    }

    if(arq_idx_point!=NULL){
        fwrite(indice_point,sizeof(id_point),p+1,arq_idx_point);
        fclose(arq_idx_point);
    }

    //liberar os malloc
    free(indice_nome);
    free(indice_point);
}
