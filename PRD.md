# G3X Louder — Product Requirements Document

**Versão:** 0.1.0  
**Status:** proposta para confirmação  
**Target:** C++20, JUCE fixado e CMake  
**Entrega inicial:** VST3 64-bit para Windows; Standalone para desenvolvimento

## 1. Visão

G3X Louder aumenta volume percebido com um macrocontrole, combinando compressão
de baixo nível, makeup e proteção de picos. Deve fortalecer tracks, buses e
mixes sem exigir configuração manual de threshold, ratio e ceiling.

A Waves descreve o OneKnob Louder como combinação de low-level compression,
peak limiting e makeup automático, com aumento de RMS de até 24 dB. O G3X usará
arquitetura e curvas próprias, priorizando segurança e medição transparente.

## 2. Objetivos

- Elevar loudness percebido mantendo picos sob controle.
- Reforçar detalhes baixos sem amplificar silêncio ou ruído indefinidamente.
- Produzir progressão musical de ganho entre 0 e 10.
- Evitar pumping, distorção e overs intersample inesperados.
- Suportar mono e estéreo linkado, automação e restauração de estado.

## 3. Controle público

### Louder (`amount`)

- Faixa exibida: 0.0–10.0; padrão 0.0; normalização interna 0–1.
- 0 é neutro; valores crescentes elevam compressão de baixo nível, makeup e
  ação do limiter.
- Alvo próprio inicial: até +18 dB de aumento nominal, podendo chegar a +24 dB
  somente após testes de ruído, headroom e qualidade.
- Automação suavizada e reset por duplo clique.

### Ceiling (`ceilingMode`)

- Opção simples proposta: `Safe` (-1 dBTP) e `Hot` (-0.1 dBFS).
- `Safe` como padrão. A necessidade de expor esse controle será validada no M0.

## 4. DSP proposto

```text
Input -> análise de nível -> upward/low-level compressor limitado
      -> makeup progressivo -> peak/true-peak limiter -> Output
```

- Expansão de detalhes por upward compression com piso de atividade para não
  elevar silêncio continuamente.
- Detector RMS/peak program-dependent e link estéreo.
- Ganho máximo limitado pelo macrocontrole e por um noise gate suave interno.
- Limiter final com release adaptativo; modo Safe pode usar oversampling e
  lookahead, sempre reportando a latência ao host.
- Mudanças de ganho suavizadas e bypass sem clique.
- Parâmetros internos serão derivados por testes; não se presume a curva Waves.

Hipóteses G3X iniciais:

- Ganho nominal: 0 a +18 dB, curva com maior precisão até o valor 5.
- Compressão low-level máxima: aproximadamente 12 dB.
- Ceiling Safe: -1 dBTP; lookahead inicial de 1 ms.
- Piso de atividade inicial: -70 dBFS com histerese.

## 5. Interface

- Knob central 0–10 e valor numérico.
- Medidores compactos de entrada, saída e redução/limitação.
- Alerta de atividade do limiter e indicador de latência no modo Safe.
- Janela compacta, redimensionável, HiDPI e acessível.
- Linguagem G3X original; nenhum elemento gráfico da Waves será reutilizado.

## 6. Requisitos de tempo real

- Sem alocações, locks, I/O ou UI em `processBlock`.
- 44.1–192 kHz; buffers de 16–2048 samples.
- Proteção contra NaN, Inf e denormals; estado versionado.
- Latência correta e atualizada quando a topologia mudar.
- Saída estéreo linkada e sem deslocamento de imagem.

## 7. Testes e aceitação

- Identidade/null test em `amount = 0`.
- Curvas de ganho, piso de atividade, ceiling e release testados.
- Loudness LUFS/RMS e true peak medidos nos pontos 0, 2.5, 5, 7.5 e 10.
- Silêncio não deve crescer; saída sempre finita; automação sem clicks.
- Builds Linux Debug/Release e Windows VST3 Release via MSVC.
- Pluginval/VST3 Validator e restauração de estado antes do beta.
- Validação no FL Studio com vocal, drums, mix e sinais de teste.

## 8. Presets iniciais

- Neutral, Vocal Lift, Drum Power, Bus Density, Mix Safe e Maximum Impact.

## 9. Fora do escopo

- Clonagem sample-a-sample ou engenharia reversa da Waves.
- Assets, marca, presets ou trade dress da Waves.
- Multibanda, mid/side, sidechain externo, AAX e iOS no primeiro release.

## 10. Marcos

1. **M0:** PRD, naming, licença e arquitetura.
2. **M1:** low-level compressor, medição e testes.
3. **M2:** makeup, limiter, estéreo e segurança.
4. **M3:** interface, medidores, presets e acessibilidade.
5. **M4:** CI Windows, artefato VST3 e FL Studio.
6. **M5:** true peak, validadores, regressão e beta.

## 11. Decisões para confirmação

- Nome `G3X Louder`; ganho máximo de +18 ou +24 dB.
- Expor Safe/Hot ou fixar ceiling seguro.
- Priorizar zero latência ou true-peak com lookahead.
- Interface com medidores ou literalmente apenas um knob.

## 12. Fontes

- [Produto oficial](https://www.waves.com/plugins/oneknob-louder)
- [Manual oficial OneKnob](https://assets.wavescdn.com/pdf/plugins/oneknob-series.pdf)
- [Imagem oficial](https://media.wavescdn.com/images/products/plugins/600/oneknob-louder.png)

Consulta em 4 de setembro de 2026; fontes usadas apenas como referência.

