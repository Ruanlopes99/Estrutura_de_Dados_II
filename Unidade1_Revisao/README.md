REGRAS DE NEGÓCIO: SISTEMA DE GERENCIAMENTO DE TRIAGEM HOSPITALAR (MÉTODO MANCHESTER ADAPTADO) 

OBJETIVO DO SISTEMA

O sistema visa gerenciar o fluxo de atendimento de pacientes na unidade hospitalar a partir da classificação de risco realizada pela equipe de enfermagem, mitigando riscos de agravamento de quadros clínicos críticos através da priorização algorítmica. 
CLASSIFICAÇÃO DE RISCO E ESTRUTURA DE DADOS.

O sistema opera com três níveis hierárquicos de classificação, cada um representado por uma estrutura de Fila Estrita do tipo FIFO (First-In, First-Out), onde o primeiro a entrar é obrigatoriamente o primeiro a ser tratado dentro de sua categoria: 
-- Emergência (Fila Vermelha - Imediato): Destinada a pacientes com risco iminente de morte ou severidade clínica extrema. Esta fila possui prioridade absoluta de interrupção sobre todas as outras.
-- Urgência (Fila Amarela): Destinada a casos graves que necessitam de atendimento célere, mas que possuem estabilidade clínica temporária de curto prazo.
-- Rotina / Efetivo (Fila de Rotina): Destinada a atendimentos eletivos, consultas de rotina, agendamentos prévios ou sintomas leves. O critério único de ordenação interno é a ordem cronológica de chegada à triagem. 

REGRAS DE ATENDIMENTO E HIERARQUIA DE CONVENÇÃO (POLÍTICA DE DESPACHO)

A chamada de pacientes pelo corpo médico deve obedecer estritamente às seguintes premissas lógicas de negócio: 
Rule #1 (Prioridade Absoluta): Nenhum paciente da Fila Amarela ou da Fila de Rotina poderá ser chamado se houver pelo menos um (1) paciente aguardando na Fila Vermelha.
Rule #2 (Prioridade Secundária): Os pacientes da Fila Amarela só receberão atendimento se a Fila Vermelha estiver completamente zerada (vazia).
Rule #3 (Atendimento de Rotina): Os pacientes da Fila de Rotina só serão chamados se, e somente se, as Filas Vermelha e Amarela estiverem simultaneamente vazias.
Rule #4 (Justiça Interna/Isocronismo): Dentro de uma mesma fila (mesma cor de classificação), o critério de desempate é estritamente a ordem de chegada (First-Come, First-Served). O paciente que passou pela triagem há mais tempo na respectiva categoria assume o topo da sublista. 
COMPORTAMENTO DILIGENTE DO FLUXO (ESTRUTURA COMPUTACIONAL)
Para evitar a realocação massiva de nós na memória interna (o que geraria perda de performance computacional em sistemas de grande escala O(N)), o software utiliza apontadores estáticos para o 'Início' e 'Fim' de três instâncias de listas encadeadas simples. O tempo de complexidade para inserção de um paciente ou para a chamada do próximo da vez é mantido em tempo constante O(1). 