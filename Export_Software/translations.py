from __future__ import annotations

_current_language: str = "English"

STRINGS: dict[str, dict[str, str]] = {
    # ── App chrome ──────────────────────────────────────────────
    "app_title": {
        "English": "EOL Export Software",
        "Spanish": "Software de Exportación EOL",
        "Hmong": "Software Xa Tawm EOL",
    },
    "eyebrow": {
        "English": "RITE-HITE CONNECTED SYSTEMS",
        "Spanish": "RITE-HITE SISTEMAS CONECTADOS",
        "Hmong": "RITE-HITE TSHUAB TXUAS",
    },
    "hero_title": {
        "English": "EOL Test Fixture Console",
        "Spanish": "Consola de Banco de Pruebas EOL",
        "Hmong": "EOL Rooj Sim Kev Kuaj Console",
    },
    "hero_subtitle": {
        "English": "Extract test results and manage Rite-Hite Connect firmware through the test fixture.",
        "Spanish": "Extraiga resultados de pruebas y administre el firmware Rite-Hite Connect a través del banco de pruebas.",
        "Hmong": "Rho cov txiaj ntsig kev sim thiab tswj Rite-Hite Connect firmware los ntawm lub rooj sim kev kuaj.",
    },

    # ── Tabs ────────────────────────────────────────────────────
    "tab_file_explorer": {
        "English": "File Explorer",
        "Spanish": "Explorador de Archivos",
        "Hmong": "Saib Cov Ntaub Ntawv",
    },
    "tab_device_tools": {
        "English": "Device Tools",
        "Spanish": "Herramientas del Dispositivo",
        "Hmong": "Cuab Yeej Khoom Siv",
    },

    # ── Status panel ────────────────────────────────────────────
    "status": {
        "English": "Status",
        "Spanish": "Estado",
        "Hmong": "Xwm Txheej",
    },
    "copy": {
        "English": "Copy",
        "Spanish": "Copiar",
        "Hmong": "Luam",
    },
    "clear": {
        "English": "Clear",
        "Spanish": "Limpiar",
        "Hmong": "Tshem Tawm",
    },
    "copy_log_tooltip": {
        "English": "Copy log to clipboard",
        "Spanish": "Copiar registro al portapapeles",
        "Hmong": "Luam cov ntaub ntawv teev tseg rau clipboard",
    },
    "clear_log_tooltip": {
        "English": "Clear the status log",
        "Spanish": "Limpiar el registro de estado",
        "Hmong": "Tshem tawm cov ntaub ntawv teev xwm txheej",
    },
    "ready_message": {
        "English": "Ready. Connect the test fixture USB to extract test results, or connect the programmer to upload Rite-Hite Connect test firmware.",
        "Spanish": "Listo. Conecte el USB del banco de pruebas para extraer resultados, o conecte el programador para cargar el firmware de prueba Rite-Hite Connect.",
        "Hmong": "Npaj txhij. Txuas lub rooj sim kev kuaj USB los rho cov txiaj ntsig, lossis txuas lub programmer los tso Rite-Hite Connect firmware kev sim.",
    },

    # ── File Explorer tab ───────────────────────────────────────
    "refresh_list": {
        "English": "Refresh List",
        "Spanish": "Actualizar Lista",
        "Hmong": "Tshiab Daim Ntawv Teev",
    },
    "open_exports_folder": {
        "English": "Open Exports Folder",
        "Spanish": "Abrir Carpeta de Exportaciones",
        "Hmong": "Qhib Lub Nplaub Tshev Xa Tawm",
    },
    "change_folder": {
        "English": "Change Folder",
        "Spanish": "Cambiar Carpeta",
        "Hmong": "Hloov Nplaub Tshev",
    },
    "reset_to_default": {
        "English": "Reset to Default",
        "Spanish": "Restablecer por Defecto",
        "Hmong": "Rov Qab Rau Qhov Qub",
    },
    "search_by_filename": {
        "English": "Search by filename\u2026",
        "Spanish": "Buscar por nombre de archivo\u2026",
        "Hmong": "Tshawb nrhiav raws lub npe ntaub ntawv\u2026",
    },
    "outcome": {
        "English": "Outcome",
        "Spanish": "Resultado",
        "Hmong": "Qhov Tshwm Sim",
    },
    "clear_filters": {
        "English": "Clear filters",
        "Spanish": "Limpiar filtros",
        "Hmong": "Tshem cov lim",
    },
    "col_filename": {
        "English": "Filename",
        "Spanish": "Nombre de Archivo",
        "Hmong": "Lub Npe Ntaub Ntawv",
    },
    "col_saved_time": {
        "English": "Saved Time",
        "Spanish": "Hora Guardado",
        "Hmong": "Lub Sij Hawm Khaws",
    },
    "col_outcome": {
        "English": "Outcome",
        "Spanish": "Resultado",
        "Hmong": "Qhov Tshwm Sim",
    },
    "no_exports_yet": {
        "English": "No exports yet",
        "Spanish": "Sin exportaciones aún",
        "Hmong": "Tseem tsis tau muaj kev xa tawm",
    },
    "no_exports_hint": {
        "English": "When a board test finishes, use Device Tools to extract data from the test fixture. CSV files will appear here.",
        "Spanish": "Cuando termine una prueba de placa, use Herramientas del Dispositivo para extraer datos del banco de pruebas. Los archivos CSV aparecerán aquí.",
        "Hmong": "Thaum kev sim daim board tiav, siv Cuab Yeej Khoom Siv los rho cov ntaub ntawv los ntawm lub rooj sim kev kuaj. Cov ntaub ntawv CSV yuav tshwm sim ntawm no.",
    },
    "go_to_device_tools": {
        "English": "Go to Device Tools",
        "Spanish": "Ir a Herramientas del Dispositivo",
        "Hmong": "Mus Rau Cuab Yeej Khoom Siv",
    },
    "refresh_list_lower": {
        "English": "Refresh list",
        "Spanish": "Actualizar lista",
        "Hmong": "Tshiab daim ntawv teev",
    },
    "no_files_match": {
        "English": "No files match your filters",
        "Spanish": "Ningún archivo coincide con sus filtros",
        "Hmong": "Tsis muaj ntaub ntawv phim koj cov lim",
    },
    "no_files_match_hint": {
        "English": "Try clearing the search box or setting Outcome back to All.",
        "Spanish": "Intente limpiar el cuadro de búsqueda o establecer Resultado en Todos.",
        "Hmong": "Sim tshem lub thawv tshawb nrhiav lossis teeb Qhov Tshwm Sim rov qab rau Tag Nrho.",
    },
    "no_rows": {
        "English": "No rows to show",
        "Spanish": "Sin filas para mostrar",
        "Hmong": "Tsis muaj kab los qhia",
    },

    # ── Device Tools tab ────────────────────────────────────────
    "connection_readiness": {
        "English": "Connection Readiness",
        "Spanish": "Estado de Conexión",
        "Hmong": "Kev Txuas Npaj Txhij",
    },
    "last_successful_operation": {
        "English": "Last Successful Operation",
        "Spanish": "Última Operación Exitosa",
        "Hmong": "Qhov Kev Ua Tiav Zaum Kawg",
    },
    "no_operation_recorded": {
        "English": "No successful export or Rite-Hite Connect test upload has been recorded on this PC yet.",
        "Spanish": "Aún no se ha registrado ninguna exportación o carga de firmware Rite-Hite Connect exitosa en esta PC.",
        "Hmong": "Tseem tsis tau muaj kev xa tawm lossis kev tso firmware Rite-Hite Connect ua tiav teev tseg hauv lub computer no.",
    },
    "finished_board_export": {
        "English": "Finished Board Export",
        "Spanish": "Exportación de Placa Completada",
        "Hmong": "Kev Xa Tawm Daim Board Tiav",
    },
    "export_card_subtitle": {
        "English": "Pull the latest completed test run from the test fixture and save one CSV file for that tested board.",
        "Spanish": "Extraiga la última prueba completada del banco de pruebas y guarde un archivo CSV para esa placa probada.",
        "Hmong": "Rho qhov kev sim tiav tshiab tshaj plaws los ntawm lub rooj sim kev kuaj thiab khaws ib daim ntaub ntawv CSV rau daim board ntawd.",
    },
    "extract_data": {
        "English": "Extract Data from Test Fixture",
        "Spanish": "Extraer Datos del Banco de Pruebas",
        "Hmong": "Rho Cov Ntaub Ntawv Los Ntawm Rooj Sim Kev Kuaj",
    },
    "test_firmware_title": {
        "English": "Rite-Hite Connect Test Firmware",
        "Spanish": "Firmware de Prueba Rite-Hite Connect",
        "Hmong": "Rite-Hite Connect Firmware Kev Sim",
    },
    "test_firmware_subtitle": {
        "English": "Upload the fixed EOL test firmware to the Rite-Hite Connect module through the programmer.",
        "Spanish": "Cargue el firmware de prueba EOL fijo al módulo Rite-Hite Connect a través del programador.",
        "Hmong": "Tso cov firmware kev sim EOL mus rau Rite-Hite Connect module los ntawm lub programmer.",
    },
    "upload_test_firmware": {
        "English": "Upload Test Firmware",
        "Spanish": "Cargar Firmware de Prueba",
        "Hmong": "Tso Firmware Kev Sim",
    },
    "production_firmware_title": {
        "English": "Rite-Hite Connect Production Firmware",
        "Spanish": "Firmware de Producción Rite-Hite Connect",
        "Hmong": "Rite-Hite Connect Firmware Tsim Khoom",
    },
    "production_firmware_subtitle": {
        "English": "Upload the final production firmware to the Rite-Hite Connect module after all tests have passed.",
        "Spanish": "Cargue el firmware de producción final al módulo Rite-Hite Connect después de que se hayan aprobado todas las pruebas.",
        "Hmong": "Tso cov firmware tsim khoom zaum kawg mus rau Rite-Hite Connect module tom qab tag nrho kev sim dhau lawm.",
    },
    "upload_production_firmware": {
        "English": "Upload Production Firmware",
        "Spanish": "Cargar Firmware de Producción",
        "Hmong": "Tso Firmware Tsim Khoom",
    },
    "coming_soon": {
        "English": "Coming Soon",
        "Spanish": "Próximamente",
        "Hmong": "Yuav Los Sai",
    },
    "coming_soon_body": {
        "English": "Production firmware upload is not available yet.\n\nThis feature is being prepared for a future release.",
        "Spanish": "La carga de firmware de producción aún no está disponible.\n\nEsta función se está preparando para una versión futura.",
        "Hmong": "Kev tso firmware tsim khoom tseem tsis tau muaj.\n\nQhov no tab tom npaj rau kev tso tawm yav tom ntej.",
    },

    # ── Workflow banner ─────────────────────────────────────────
    "wf_connect_fixture": {
        "English": "Connect the test fixture USB cable to get started.",
        "Spanish": "Conecte el cable USB del banco de pruebas para comenzar.",
        "Hmong": "Txuas lub rooj sim kev kuaj USB cable los pib.",
    },
    "wf_fixture_ready": {
        "English": "Fixture ready. Connect the Rite-Hite Connect programmer to upload firmware.",
        "Spanish": "Banco listo. Conecte el programador Rite-Hite Connect para cargar firmware.",
        "Hmong": "Rooj sim npaj txhij. Txuas Rite-Hite Connect programmer los tso firmware.",
    },
    "wf_all_ready": {
        "English": "Ready. Extract test data or upload firmware below.",
        "Spanish": "Listo. Extraiga datos de prueba o cargue firmware a continuación.",
        "Hmong": "Npaj txhij. Rho cov ntaub ntawv kev sim lossis tso firmware hauv qab no.",
    },

    # ── Connection status ───────────────────────────────────────
    "fixture_no_ports": {
        "English": "Test Fixture: No Serial Ports Detected on This PC.",
        "Spanish": "Banco de Pruebas: No Se Detectaron Puertos Seriales en Esta PC.",
        "Hmong": "Rooj Sim Kev Kuaj: Tsis Pom Cov Serial Ports Hauv Lub Computer No.",
    },
    "fixture_not_detected": {
        "English": "Test Fixture: Not Detected. Check the USB Cable and Driver.",
        "Spanish": "Banco de Pruebas: No Detectado. Revise el Cable USB y el Controlador.",
        "Hmong": "Rooj Sim Kev Kuaj: Tsis Pom. Kuaj Lub USB Cable Thiab Driver.",
    },
    "fixture_detected": {
        "English": "Test Fixture: Detected on {port}.",
        "Spanish": "Banco de Pruebas: Detectado en {port}.",
        "Hmong": "Rooj Sim Kev Kuaj: Pom Ntawm {port}.",
    },
    "fixture_multiple": {
        "English": "Test Fixture: {count} Candidate Ports — The App May Prompt You to Choose One.",
        "Spanish": "Banco de Pruebas: {count} Puertos Candidatos — La Aplicación Puede Pedirle Que Elija Uno.",
        "Hmong": "Rooj Sim Kev Kuaj: {count} Lub Ports — Lub App Yuav Nug Koj Xaiv Ib Qho.",
    },
    "module_prefix": {
        "English": "Rite-Hite Connect Module: {message}",
        "Spanish": "Módulo Rite-Hite Connect: {message}",
        "Hmong": "Rite-Hite Connect Module: {message}",
    },

    # ── Extract / upload messages ───────────────────────────────
    "confirm_extract_title": {
        "English": "Confirm Extract",
        "Spanish": "Confirmar Extracción",
        "Hmong": "Paub Meej Rho Tawm",
    },
    "confirm_extract_body": {
        "English": "Extract the latest test run from the fixture?",
        "Spanish": "¿Extraer la última ejecución de prueba del banco?",
        "Hmong": "Rho qhov kev sim tshiab tshaj plaws los ntawm lub rooj sim?",
    },
    "export_cancelled": {
        "English": "Export cancelled. No serial port was selected.",
        "Spanish": "Exportación cancelada. No se seleccionó un puerto serial.",
        "Hmong": "Kev xa tawm raug tso tseg. Tsis tau xaiv serial port.",
    },
    "export_connecting": {
        "English": "Connected to {port}. Requesting export status from the test fixture.",
        "Spanish": "Conectado a {port}. Solicitando estado de exportación del banco de pruebas.",
        "Hmong": "Txuas rau {port}. Thov xwm txheej kev xa tawm los ntawm lub rooj sim kev kuaj.",
    },
    "export_saved": {
        "English": "Saved run {seq} ({outcome}) to {name}.",
        "Spanish": "Ejecución {seq} ({outcome}) guardada en {name}.",
        "Hmong": "Khaws kev sim {seq} ({outcome}) rau {name}.",
    },
    "export_saved_flash": {
        "English": "Export saved successfully — {name}",
        "Spanish": "Exportación guardada exitosamente — {name}",
        "Hmong": "Kev xa tawm khaws tau zoo — {name}",
    },
    "export_failed_flash": {
        "English": "Export failed — {reason}",
        "Spanish": "Exportación fallida — {reason}",
        "Hmong": "Kev xa tawm tsis tiav — {reason}",
    },
    "export_failed_title": {
        "English": "Export Failed",
        "Spanish": "Exportación Fallida",
        "Hmong": "Kev Xa Tawm Tsis Tiav",
    },
    "export_firmware_blocked": {
        "English": "The test fixture firmware rejected the re-export. The fixture may need to run a new test before exporting again.",
        "Spanish": "El firmware del banco rechazó la re-exportación. Es posible que el banco necesite ejecutar una nueva prueba antes de exportar nuevamente.",
        "Hmong": "Lub rooj sim firmware tsis lees txais kev rov xa tawm. Lub rooj sim yuav tsum tau khiav kev sim tshiab ua ntej xa tawm dua.",
    },
    "export_firmware_blocked_flash": {
        "English": "Export blocked by fixture firmware — run a new test first",
        "Spanish": "Exportación bloqueada por el firmware del banco — ejecute una nueva prueba primero",
        "Hmong": "Kev xa tawm raug thaiv los ntawm firmware — khiav kev sim tshiab ua ntej",
    },
    "upload_cancelled": {
        "English": "Rite-Hite Connect test firmware upload cancelled.",
        "Spanish": "Carga de firmware de prueba Rite-Hite Connect cancelada.",
        "Hmong": "Kev tso Rite-Hite Connect firmware kev sim raug tso tseg.",
    },
    "upload_success_flash": {
        "English": "Firmware uploaded successfully",
        "Spanish": "Firmware cargado exitosamente",
        "Hmong": "Firmware tso tau zoo",
    },
    "upload_failed_flash": {
        "English": "Upload failed — {reason}",
        "Spanish": "Carga fallida — {reason}",
        "Hmong": "Kev tso tsis tiav — {reason}",
    },
    "upload_failed_log": {
        "English": "Rite-Hite Connect upload failed: {reason}",
        "Spanish": "Carga Rite-Hite Connect fallida: {reason}",
        "Hmong": "Rite-Hite Connect kev tso tsis tiav: {reason}",
    },
    "upload_setup_failed": {
        "English": "Rite-Hite Connect upload setup failed: {reason}",
        "Spanish": "Configuración de carga Rite-Hite Connect fallida: {reason}",
        "Hmong": "Rite-Hite Connect kev teeb tsa tso tsis tiav: {reason}",
    },
    "no_programmer_detected": {
        "English": "No programmer serial ports were detected. Connect the programmer and try again.",
        "Spanish": "No se detectaron puertos seriales del programador. Conecte el programador e intente de nuevo.",
        "Hmong": "Tsis pom programmer serial ports. Txuas lub programmer thiab sim dua.",
    },

    # ── Context menu ────────────────────────────────────────────
    "ctx_open_file": {
        "English": "Open file",
        "Spanish": "Abrir archivo",
        "Hmong": "Qhib ntaub ntawv",
    },
    "ctx_reveal_folder": {
        "English": "Reveal in folder",
        "Spanish": "Mostrar en carpeta",
        "Hmong": "Qhia hauv nplaub tshev",
    },
    "ctx_copy_path": {
        "English": "Copy full path",
        "Spanish": "Copiar ruta completa",
        "Hmong": "Luam txoj kev tag nrho",
    },
    "ctx_preview_csv": {
        "English": "Preview first lines of CSV\u2026",
        "Spanish": "Vista previa de las primeras líneas del CSV\u2026",
        "Hmong": "Saib ua ntej thawj kab ntawm CSV\u2026",
    },
    "ctx_delete_file": {
        "English": "Delete file\u2026",
        "Spanish": "Eliminar archivo\u2026",
        "Hmong": "Rho tawm ntaub ntawv\u2026",
    },

    # ── Delete dialog ───────────────────────────────────────────
    "delete_export_title": {
        "English": "Delete Export",
        "Spanish": "Eliminar Exportación",
        "Hmong": "Rho Tawm Kev Xa Tawm",
    },
    "delete_export_body": {
        "English": "Permanently delete this file?\n\n{name}",
        "Spanish": "¿Eliminar permanentemente este archivo?\n\n{name}",
        "Hmong": "Rho tawm mus li daim ntaub ntawv no?\n\n{name}",
    },
    "delete_failed_title": {
        "English": "Delete Failed",
        "Spanish": "Eliminación Fallida",
        "Hmong": "Rho Tawm Tsis Tiav",
    },
    "deleted_export": {
        "English": "Deleted export {name}.",
        "Spanish": "Exportación {name} eliminada.",
        "Hmong": "Rho tawm kev xa tawm {name}.",
    },

    # ── Folder operations ───────────────────────────────────────
    "select_exports_folder": {
        "English": "Select Exports Folder",
        "Spanish": "Seleccionar Carpeta de Exportaciones",
        "Hmong": "Xaiv Lub Nplaub Tshev Xa Tawm",
    },
    "folder_changed": {
        "English": "Exports folder changed to {path}.",
        "Spanish": "Carpeta de exportaciones cambiada a {path}.",
        "Hmong": "Lub nplaub tshev xa tawm hloov mus rau {path}.",
    },
    "folder_reset": {
        "English": "Exports folder reset to default.",
        "Spanish": "Carpeta de exportaciones restablecida por defecto.",
        "Hmong": "Lub nplaub tshev xa tawm rov qab rau qhov qub.",
    },

    # ── Settings ────────────────────────────────────────────────
    "settings": {
        "English": "Settings",
        "Spanish": "Configuración",
        "Hmong": "Kev Teeb Tsa",
    },
    "settings_saved": {
        "English": "Settings saved.",
        "Spanish": "Configuración guardada.",
        "Hmong": "Kev teeb tsa khaws tseg.",
    },

    # ── ESP32 uploader (esp32_uploader.py) ────────────────────
    "esp_detected_on": {
        "English": "Programmer Detected on {port}.",
        "Spanish": "Programador Detectado en {port}.",
        "Hmong": "Programmer Pom Ntawm {port}.",
    },
    "esp_detected_pair": {
        "English": "Programmer Pair Detected: {ports}.",
        "Spanish": "Par de Programador Detectado: {ports}.",
        "Hmong": "Pom Programmer Khub: {ports}.",
    },
    "esp_multiple_candidates": {
        "English": "Multiple Programmer Candidates Found ({count}) — The App Will Ask You to Choose.",
        "Spanish": "Se Encontraron Múltiples Candidatos de Programador ({count}) — La Aplicación Le Pedirá Que Elija.",
        "Hmong": "Pom Ntau Tus Programmer ({count}) — Lub App Yuav Nug Koj Xaiv.",
    },
    "esp_not_detected": {
        "English": "Programmer Not Detected. Connect the Programming Cable and Try Again.",
        "Spanish": "Programador No Detectado. Conecte el Cable de Programación e Intente de Nuevo.",
        "Hmong": "Tsis Pom Programmer. Txuas Txoj Hlua Programmer Thiab Sim Dua.",
    },
    "esp_no_pair_reason": {
        "English": "No programmer pair was detected automatically. Select the upload port manually.",
        "Spanish": "No se detectó ningún par de programador automáticamente. Seleccione el puerto de carga manualmente.",
        "Hmong": "Tsis pom programmer khub cia li. Xaiv lub upload port koj tus kheej.",
    },
    "esp_multi_pair_reason": {
        "English": "Multiple programmer candidates found — choose the correct upload port.",
        "Spanish": "Se encontraron múltiples candidatos de programador — elija el puerto de carga correcto.",
        "Hmong": "Pom ntau tus programmer — xaiv lub upload port raug.",
    },

    # ── Last-operation summary (storage.py) ───────────────────
    "last_op_export": {
        "English": "Export — {ts}\nSaved: {name}\nPort: {device}",
        "Spanish": "Exportación — {ts}\nGuardado: {name}\nPuerto: {device}",
        "Hmong": "Xa Tawm — {ts}\nKhaws: {name}\nPort: {device}",
    },
    "last_op_upload": {
        "English": "Rite-Hite Connect Upload — {ts}\nPort: {device}\n{detail}",
        "Spanish": "Carga Rite-Hite Connect — {ts}\nPuerto: {device}\n{detail}",
        "Hmong": "Rite-Hite Connect Kev Tso — {ts}\nPort: {device}\n{detail}",
    },

    # ── Settings dialog ──────────────────────────────────────────
    "settings_appearance": {
        "English": "Appearance",
        "Spanish": "Apariencia",
        "Hmong": "Qhov Zoo Nkauj",
    },
    "settings_dark_mode": {
        "English": "Enable Dark Mode",
        "Spanish": "Activar Modo Oscuro",
        "Hmong": "Qhib Hom Tsaus",
    },
    "settings_font_size": {
        "English": "Font Size",
        "Spanish": "Tamaño de Fuente",
        "Hmong": "Loj Ntawv",
    },
    "settings_language_section": {
        "English": "Language",
        "Spanish": "Idioma",
        "Hmong": "Lus",
    },
    "settings_display_language": {
        "English": "Display Language",
        "Spanish": "Idioma de Visualización",
        "Hmong": "Lus Qhia",
    },
    "settings_language_note": {
        "English": "More languages coming in a future release.",
        "Spanish": "Más idiomas próximamente en una futura versión.",
        "Hmong": "Ntau hom lus yuav los sai sai no.",
    },
    "settings_behavior": {
        "English": "Behavior",
        "Spanish": "Comportamiento",
        "Hmong": "Kev Coj Cwj Pwm",
    },
    "settings_confirm_extract": {
        "English": "Confirm Before Extracting Data",
        "Spanish": "Confirmar Antes de Extraer Datos",
        "Hmong": "Paub Meej Ua Ntej Rho Cov Ntaub Ntawv",
    },
    "settings_auto_refresh": {
        "English": "Auto-Refresh on Tab Switch",
        "Spanish": "Actualización Automática al Cambiar de Pestaña",
        "Hmong": "Cia Li Tshiab Thaum Hloov Tab",
    },
    "settings_reduced_motion": {
        "English": "Reduce Motion (disable animations)",
        "Spanish": "Reducir Movimiento (desactivar animaciones)",
        "Hmong": "Txo Kev Txav (tua cov animations)",
    },
    "activity_log": {
        "English": "Activity Log",
        "Spanish": "Registro de Actividad",
        "Hmong": "Ntaub Ntawv Teev Kev Ua Hauj Lwm",
    },
    "nav_settings": {
        "English": "Settings",
        "Spanish": "Configuración",
        "Hmong": "Kev Teeb Tsa",
    },
    "nav_collapse": {
        "English": "Collapse",
        "Spanish": "Colapsar",
        "Hmong": "Kaw",
    },
    "nav_expand": {
        "English": "Expand",
        "Spanish": "Expandir",
        "Hmong": "Nthuav",
    },
    "show_activity_log": {
        "English": "Show activity log",
        "Spanish": "Mostrar registro de actividad",
        "Hmong": "Qhia ntaub ntawv teev kev ua hauj lwm",
    },
    "hide_activity_log": {
        "English": "Hide activity log",
        "Spanish": "Ocultar registro de actividad",
        "Hmong": "Zais ntaub ntawv teev kev ua hauj lwm",
    },
    "cancel": {
        "English": "Cancel",
        "Spanish": "Cancelar",
        "Hmong": "Tso Tseg",
    },
    "save": {
        "English": "Save",
        "Spanish": "Guardar",
        "Hmong": "Khaws",
    },

    # ── Tooltips ────────────────────────────────────────────────
    "tooltip_refresh": {
        "English": "Refresh file list (F5)",
        "Spanish": "Actualizar lista de archivos (F5)",
        "Hmong": "Tshiab daim ntawv teev cov ntaub ntawv (F5)",
    },
    "tooltip_open_folder": {
        "English": "Open exports folder (Ctrl+O)",
        "Spanish": "Abrir carpeta de exportaciones (Ctrl+O)",
        "Hmong": "Qhib lub nplaub tshev xa tawm (Ctrl+O)",
    },
    "tooltip_change_folder": {
        "English": "Choose a different folder for exports",
        "Spanish": "Elegir una carpeta diferente para exportaciones",
        "Hmong": "Xaiv lwm lub nplaub tshev rau kev xa tawm",
    },
    "tooltip_reset_folder": {
        "English": "Revert to the default exports folder",
        "Spanish": "Volver a la carpeta de exportaciones por defecto",
        "Hmong": "Rov qab mus rau lub nplaub tshev xa tawm qub",
    },
    "tooltip_extract": {
        "English": "Extract Data from Test Fixture (Ctrl+E)",
        "Spanish": "Extraer Datos del Banco de Pruebas (Ctrl+E)",
        "Hmong": "Rho Cov Ntaub Ntawv Los Ntawm Rooj Sim Kev Kuaj (Ctrl+E)",
    },
    "tooltip_upload_test": {
        "English": "Upload Test Firmware (Ctrl+U)",
        "Spanish": "Cargar Firmware de Prueba (Ctrl+U)",
        "Hmong": "Tso Firmware Kev Sim (Ctrl+U)",
    },
    "tooltip_upload_production": {
        "English": "Upload production firmware to the Rite-Hite Connect module",
        "Spanish": "Cargar firmware de producción al módulo Rite-Hite Connect",
        "Hmong": "Tso firmware tsim khoom mus rau Rite-Hite Connect module",
    },
    "tooltip_delete": {
        "English": "Delete this export file",
        "Spanish": "Eliminar este archivo de exportación",
        "Hmong": "Rho tawm daim ntaub ntawv xa tawm no",
    },
    "tooltip_settings": {
        "English": "Settings",
        "Spanish": "Configuración",
        "Hmong": "Kev Teeb Tsa",
    },

    # ── One Click Test tab ───────────────────────────────────────
    "tab_one_click_test": {
        "English": "One Click Test",
        "Spanish": "Prueba con Un Clic",
        "Hmong": "Kev Sim Ib Nyem",
    },
    "oct_title": {
        "English": "One Click Automatic Test",
        "Spanish": "Prueba Automática con Un Clic",
        "Hmong": "Kev Sim Cia Li Ib Nyem",
    },
    "oct_subtitle": {
        "English": "Upload firmware, run the full automatic test, wait for results, and extract the CSV — all in one click.",
        "Spanish": "Cargue el firmware, ejecute la prueba automática completa, espere los resultados y extraiga el CSV — todo con un clic.",
        "Hmong": "Tso firmware, khiav kev sim tag nrho, tos cov txiaj ntsig, thiab rho CSV — tag nrho hauv ib qho nyem.",
    },
    "oct_start_btn": {
        "English": "Start One Click Test",
        "Spanish": "Iniciar Prueba con Un Clic",
        "Hmong": "Pib Kev Sim Ib Nyem",
    },
    "oct_start_tooltip": {
        "English": "Run the full automated test workflow in one click",
        "Spanish": "Ejecutar todo el flujo de prueba automatizado con un clic",
        "Hmong": "Khiav tag nrho kev sim cia li hauv ib qho nyem",
    },
    "oct_step_upload": {
        "English": "Upload Firmware",
        "Spanish": "Cargar Firmware",
        "Hmong": "Tso Firmware",
    },
    "oct_step_comms": {
        "English": "Comms",
        "Spanish": "Comunicacion",
        "Hmong": "Kev Sib Txuas",
    },
    "oct_step_digital_inputs": {
        "English": "Digital Inputs",
        "Spanish": "Entradas Digitales",
        "Hmong": "Digital Inputs",
    },
    "oct_step_analog_inputs": {
        "English": "Analog Inputs",
        "Spanish": "Entradas Analogicas",
        "Hmong": "Analog Inputs",
    },
    "oct_step_digital_outputs": {
        "English": "Digital Outputs",
        "Spanish": "Salidas Digitales",
        "Hmong": "Digital Outputs",
    },
    "oct_step_relay_outputs": {
        "English": "Relay Outputs",
        "Spanish": "Salidas de Rele",
        "Hmong": "Relay Outputs",
    },
    "oct_step_buttons": {
        "English": "Buttons",
        "Spanish": "Botones",
        "Hmong": "Buttons",
    },
    "oct_step_leds": {
        "English": "LEDs",
        "Spanish": "LEDs",
        "Hmong": "LEDs",
    },
    "oct_step_test": {
        "English": "Run Test",
        "Spanish": "Ejecutar Prueba",
        "Hmong": "Khiav Kev Sim",
    },
    "oct_step_wait": {
        "English": "Wait for Results",
        "Spanish": "Esperar Resultados",
        "Hmong": "Tos Cov Txiaj Ntsig",
    },
    "oct_step_extract": {
        "English": "Extract Data",
        "Spanish": "Extraer Datos",
        "Hmong": "Rho Ntaub Ntawv",
    },
    "oct_stage_uploading": {
        "English": "Uploading test firmware to Rite-Hite Connect module\u2026",
        "Spanish": "Cargando firmware de prueba al módulo Rite-Hite Connect\u2026",
        "Hmong": "Tso firmware kev sim mus rau Rite-Hite Connect module\u2026",
    },
    "oct_upload_done": {
        "English": "Firmware upload complete.",
        "Spanish": "Carga de firmware completada.",
        "Hmong": "Kev tso firmware tiav.",
    },
    "oct_esp_boot_wait": {
        "English": "Waiting for Rite-Hite Connect module to boot\u2026",
        "Spanish": "Esperando a que el m\u00f3dulo Rite-Hite Connect inicie\u2026",
        "Hmong": "Tos Rite-Hite Connect module pib\u2026",
    },
    "oct_stage_starting": {
        "English": "Sending START_AUTO command to test fixture\u2026",
        "Spanish": "Enviando comando START_AUTO al banco de pruebas\u2026",
        "Hmong": "Xa lus txib START_AUTO mus rau lub rooj sim kev kuaj\u2026",
    },
    "oct_auto_started": {
        "English": "Automatic test started on the fixture.",
        "Spanish": "Prueba automática iniciada en el banco.",
        "Hmong": "Kev sim cia li pib ntawm lub rooj sim.",
    },
    "oct_stage_waiting": {
        "English": "Waiting for the automatic test to complete\u2026",
        "Spanish": "Esperando a que la prueba automática se complete\u2026",
        "Hmong": "Tos kev sim cia li kom tiav\u2026",
    },
    "oct_visual_prompt": {
        "English": "Technician action required: on the fixture, press LED's GOOD or LED's BAD to finish the LED check.",
        "Spanish": "Se requiere acciÃ³n del tÃ©cnico: en el banco, presione LED's GOOD o LED's BAD para terminar la revisiÃ³n de LED.",
        "Hmong": "Tus technician yuav tsum ua: ntawm lub rooj sim, nias LED's GOOD lossis LED's BAD kom tiav kev kuaj LED.",
    },
    "oct_stage_extracting": {
        "English": "Extracting test data from the fixture\u2026",
        "Spanish": "Extrayendo datos de prueba del banco\u2026",
        "Hmong": "Rho cov ntaub ntawv kev sim los ntawm lub rooj sim\u2026",
    },
    "oct_timeout": {
        "English": "Automatic test timed out after 5 minutes. The fixture may need manual inspection.",
        "Spanish": "La prueba automática se agotó después de 5 minutos. El banco puede necesitar inspección manual.",
        "Hmong": "Kev sim cia li tas sij hawm tom qab 5 feeb. Lub rooj sim yuav tsum tau kuaj ntawm tus kheej.",
    },
    "oct_unexpected_state": {
        "English": "Unexpected fixture state after test: {state}",
        "Spanish": "Estado inesperado del banco después de la prueba: {state}",
        "Hmong": "Xwm txheej tsis xav txog tom qab kev sim: {state}",
    },
    "oct_complete": {
        "English": "One Click Test complete — {outcome}. Saved to {name}.",
        "Spanish": "Prueba con Un Clic completada — {outcome}. Guardado en {name}.",
        "Hmong": "Kev Sim Ib Nyem tiav — {outcome}. Khaws rau {name}.",
    },
    "oct_failed_complete": {
        "English": "One Click Test failed - {outcome}. Saved to {name}.",
        "Spanish": "La Prueba con Un Clic fallo - {outcome}. Guardado en {name}.",
        "Hmong": "Kev Sim Ib Nyem poob lawm - {outcome}. Khaws rau {name}.",
    },
    "oct_upload_busy": {
        "English": "A firmware upload is already in progress. Please wait for it to finish.",
        "Spanish": "Ya hay una carga de firmware en progreso. Espere a que termine.",
        "Hmong": "Kev tso firmware tab tom ua. Thov tos kom tiav.",
    },
    "oct_no_esp": {
        "English": "Cannot start: the programmer was not detected.",
        "Spanish": "No se puede iniciar: no se detectó el programador.",
        "Hmong": "Tsis tuaj yeem pib: tsis pom tus programmer.",
    },
    "oct_busy_locked": {
        "English": "One Click Test is already running. Wait for it to finish before using manual extract or firmware upload.",
        "Spanish": "La Prueba con Un Clic ya esta en progreso. Espere a que termine antes de usar la extraccion manual o la carga de firmware.",
        "Hmong": "Kev Sim Ib Nyem tab tom khiav. Thov tos kom tiav ua ntej siv rho ntaub ntawv manually lossis tso firmware.",
    },
    "fixture_not_found": {
        "English": "Test Fixture: Not Found. Connect the USB Cable.",
        "Spanish": "Banco de Pruebas: No Encontrado. Conecte el Cable USB.",
        "Hmong": "Rooj Sim Kev Kuaj: Tsis Pom. Txuas Lub USB Cable.",
    },
}


def set_language(lang: str) -> None:
    global _current_language
    _current_language = lang


def get_language() -> str:
    return _current_language


def tr(key: str, **kwargs: object) -> str:
    """Look up the translated string for *key* in the current language.

    Any ``{name}`` placeholders in the string are filled from *kwargs*.
    Falls back to English if the key or language is missing.
    """
    entry = STRINGS.get(key)
    if entry is None:
        return key
    text = entry.get(_current_language) or entry.get("English", key)
    if kwargs:
        try:
            text = text.format(**kwargs)
        except KeyError:
            pass
    return text
