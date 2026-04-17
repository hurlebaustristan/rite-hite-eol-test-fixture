# EOL Export Software

Desktop export tool for pulling completed NUCLEO-U575ZI-Q test runs over the ST-LINK virtual COM port and saving one CSV file per tested board.

## Install

```powershell
cd C:\TouchGFXProjects\EOL_TestFixture_Final\Export_Software
python -m pip install -r requirements.txt
```

## Run

```powershell
cd C:\TouchGFXProjects\EOL_TestFixture_Final\Export_Software
python main.py
```

## Notes

- The export path uses the STM32 `USART1` ST-LINK virtual COM port at `115200 8N1`.
- Export is available only after a test reaches a terminal pass or fail state.
- Each completed run can be exported once.
- CSV files are written to `Export_Software\exports`.
