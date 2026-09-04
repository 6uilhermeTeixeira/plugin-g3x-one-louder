# G3X One Louder

Maximizador de loudness com um macrocontrole, em desenvolvimento com C++20,
JUCE e CMake. A entrega inicial será VST3 64-bit para Windows e Standalone.

**Estado:** M3 — interface em validação. O macrocontrole combina detector RMS, upward
compression, makeup e limiter estéreo linkado com lookahead de 1 ms.

A interface M3 já está em validação, com knob central, ceilings Safe/Hot,
medidores de entrada, lift, limiter e saída, seis presets e indicação de latência.

## Build rápido

```bash
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug
```

- [PRD](PRD.md)
- [Referência visual e fontes](docs/references/README.md)
- [Marcos](docs/MILESTONES.md)
- [Validação Windows/FL Studio](docs/M4_WINDOWS_VALIDATION.md)
- [Checklist de release](docs/RELEASE_CHECKLIST.md)

![Referência Waves OneKnob Louder](docs/references/waves-oneknob-louder-interface.png)

O produto terá algoritmo, identidade, interface, textos e presets originais.

## Licença

Distribuído sob a [licença MIT](LICENSE).

## Download e instalação — Windows x64

1. Abra [Actions](https://github.com/6uilhermeTeixeira/plugin-g3x-one-louder/actions) e selecione uma execução bem-sucedida da branch `main`.
2. Em **Artifacts**, baixe `G3X-One-Louder-Windows-x64-<commit>`. O download fica disponível por 30 dias; **Run workflow** permite gerar um novo build.
3. Extraia o ZIP. A raiz contém somente `SHA256SUMS.txt` e a pasta `G3X One Louder.vst3`, com todos os arquivos internos do plugin.
4. Na pasta extraída, abra o PowerShell e verifique o binário:

```powershell
$expected, $relativePath = (Get-Content -LiteralPath .\SHA256SUMS.txt -Raw).Trim() -split '  ', 2
$actual = (Get-FileHash -LiteralPath $relativePath -Algorithm SHA256).Hash.ToLowerInvariant()
if ($actual -ne $expected) { throw "SHA-256 divergente; baixe o artifact novamente." }
"SHA-256 confirmado."
```

5. Copie a pasta **`G3X One Louder.vst3` inteira** para `C:\Program Files\Common Files\VST3` e atualize a busca de plugins da DAW. A cópia pode solicitar permissão de administrador.

O SHA-256 verifica o binário Windows x64 dentro do bundle; não é o hash do ZIP ou dos recursos. O artifact contém o VST3 Release; o aplicativo Standalone continua disponível como alvo de compilação, mas não é incluído no download.
