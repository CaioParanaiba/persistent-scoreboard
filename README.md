# Persistent Scoreboard Engine 💾🏆

> **Nota Acadêmica:** Este repositório contém um trabalho prático desenvolvido para a disciplina de **Técnicas de Programação**. O objetivo principal da implementação foi evoluir os conceitos de **alocação dinâmica** e **funções** desenvolvidos anteriormente, aprofundando intensamente na **manipulação avançada de arquivos binários** em C (`fread`, `fwrite`, `fseek`) para simular o comportamento de um SGBD real.

Um motor de base de dados em C de alta performance, desenhado para gerir tabelas de classificação (*scoreboards*) de Maratonas de Programação (estilo ICPC / Codeforces). 

Esta versão implementa uma arquitetura **Disk-Backed Storage com In-Memory Indexing**, garantindo persistência de dados em disco físico com tempos de resposta de busca em $O(\log N)$ diretamente na memória RAM.

## 🏗️ Arquitetura e Engenharia de Dados

Para evitar o consumo excessivo de memória RAM ao escalar o número de competidores, o sistema separa o armazenamento físico da indexação lógica:

* **Disk-Backed Storage (`rb+`):** Os dados pesados (nomes completos, pontuações, métricas de tempo) nunca são carregados integralmente para a RAM. Residem num ficheiro binário (`bd_competidores.bin`). A manipulação é feita através de ponteiros de ficheiro (`fseek` e `fread`/`fwrite`), carregando apenas o registo necessário para a memória num *buffer* temporário.
* **In-Memory Indexing:** Dois índices leves (ID-Nome e ID-Pontos) são carregados para a RAM no arranque do sistema. Todas as ordenações e buscas binárias ocorrem na memória primária, na velocidade do processador, descendo ao disco apenas para buscar o corpo do registo encontrado.
* **Append-Only & Soft Deletes:** Para evitar a fragmentação de ficheiros e operações custosas de reordenação em disco, as atualizações utilizam uma abordagem *Append-Only* (inserção no final do ficheiro). Registos antigos ou apagados recebem um *Soft Delete* (`ativo = 0`), tornando-se "fantasmas" ignorados pelo sistema, mas mantendo a integridade do índice.
* **Composite Primary Keys:** Para viabilizar a Busca Binária num cenário de empates (competidores com as mesmas resoluções e penalizações), o sistema gera uma chave composta matematicamente perfeita:
  `Point = (Solves * 100M) - (Penalty * 1K) - ID`

## 🚀 Funcionalidades

- **CRUD Completo Persistente:** Inserção, listagem, atualização e remoção lógica em ficheiros `.bin`.
- **Prevenção de Buffer Overflow:** Inputs sanitizados e limitados na leitura da memória (`%49s`), prevenindo *Stack Smashing*.
- **Range Queries (Busca de Intervalos):** Algoritmos de `Lower Bound` e `Upper Bound` implementados do zero para localizar rapidamente lotes de competidores dentro de um intervalo específico de problemas resolvidos.
- **Listagem Paginada Dinâmica:** Leituras sequenciais do disco através de ponteiros indexados, permitindo ordenação alfabética ou por *ranking* sem sobrecarregar a memória RAM.

## 🛠️ Como Compilar e Executar

Sendo um projeto nativo em C, a compilação requer o `gcc` (GNU Compiler Collection).

```bash
# Clonar o repositório
git clone [https://github.com/teu-usuario/persistent-scoreboard.git](https://github.com/teu-usuario/persistent-scoreboard.git)
cd persistent-scoreboard

# Compilar o código fonte
gcc main.c -o persistent_db -O2

# Executar o motor de base de dados
./persistent_db
