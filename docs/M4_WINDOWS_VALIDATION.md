# Validação Windows Alpha

## Artefato e instalação

1. Envie o commit e aguarde o workflow `Build Windows VST3`.
2. Confirme os dois testes CTest e baixe o artefato da execução.
3. Verifique o binário usando `SHA256SUMS.txt` e `Get-FileHash`.
4. Copie o bundle para `C:\Program Files\Common Files\VST3`.
5. No FL Studio, execute `Options > Manage plugins > Find installed plugins`.

## Matriz no FL Studio

Teste em 44,1 e 48 kHz, com buffers de 64, 256 e 1024 amostras:

- Instâncias mono e estéreo.
- Amount 0 com null test compensando a latência reportada.
- Automação lenta e rápida de 0→10→0.
- Bypass durante áudio sustentado e transientes.
- Alternância Safe/Hot durante reprodução.
- Todos os presets e atualização visual dos controles.
- Save/reload do projeto com amount, ceiling, bypass e preset alterados.
- Confirmação de 1 ms de latência e alinhamento com outras tracks.
- Redimensionamento, foco por teclado e leitura HiDPI.
- Ausência de clicks, pumping anormal, divergência L/R e saída não finita.

## Escuta

Use vocal, bateria, bus e mix completa. Registre loudness antes/depois, pico,
faixa útil do knob, pumping, ruído elevado e distorção. Compare Safe e Hot com
loudness aproximado. Valores de 7,5–10 exigem atenção especial a transientes.

## Validadores

Rode pluginval em strictness 5 e Steinberg VST3 Validator. Crash, erro de estado,
bus inválido, latência incorreta ou áudio não finito bloqueiam o alpha.

M4 termina após aprovação da matriz em uma máquina Windows x64, com versão do
FL Studio, logs, checksum e observações auditivas anexados à release.
