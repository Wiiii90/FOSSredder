[Code]
const
  AppUninstallKey = 'Software\Microsoft\Windows\CurrentVersion\Uninstall\{D597F3F2-8127-43F7-9B0D-7EF1B3B85836}_is1';

function QueryUninstallString(RootKey: Integer; var UninstallString: String): Boolean;
begin
  Result := RegQueryStringValue(RootKey, AppUninstallKey, 'UninstallString', UninstallString);
end;

function FindExistingUninstaller(var UninstallString: String): Boolean;
begin
  Result := QueryUninstallString(HKLM, UninstallString);

  if not Result then
    Result := QueryUninstallString(HKCU, UninstallString);
end;

function ExtractExecutablePath(CommandLine: String): String;
var
  EndQuote: Integer;
  FirstSpace: Integer;
begin
  CommandLine := Trim(CommandLine);

  if Copy(CommandLine, 1, 1) = '"' then
  begin
    Delete(CommandLine, 1, 1);
    EndQuote := Pos('"', CommandLine);

    if EndQuote > 0 then
      Result := Copy(CommandLine, 1, EndQuote - 1)
    else
      Result := CommandLine;
  end
  else
  begin
    FirstSpace := Pos(' ', CommandLine);

    if FirstSpace > 0 then
      Result := Copy(CommandLine, 1, FirstSpace - 1)
    else
      Result := CommandLine;
  end;
end;

function RunExistingUninstaller(UninstallString: String): Boolean;
var
  ResultCode: Integer;
  Uninstaller: String;
begin
  Uninstaller := ExtractExecutablePath(UninstallString);

  if not FileExists(Uninstaller) then
  begin
    Result := False;
    Exit;
  end;

  Result := Exec(Uninstaller, '', '', SW_SHOWNORMAL, ewWaitUntilTerminated, ResultCode);
end;

function InitializeSetup(): Boolean;
var
  Choice: Integer;
  UninstallString: String;
begin
  Result := True;

  if WizardSilent() then
    Exit;

  if not FindExistingUninstaller(UninstallString) then
    Exit;

  Choice := TaskDialogMsgBox(ExpandConstant('{cm:MaintenanceAlreadyInstalled}'),
    ExpandConstant('{cm:MaintenanceChoice}'), mbConfirmation, MB_YESNOCANCEL, [
    ExpandConstant('{cm:MaintenanceRepairButton}'),
    ExpandConstant('{cm:MaintenanceUninstallButton}')], IDYES);

  case Choice of
    IDYES:
      Result := True;
    IDNO:
      begin
        Result := False;

        if not RunExistingUninstaller(UninstallString) then
          MsgBox(ExpandConstant('{cm:MaintenanceUninstallFailed}'), mbError, MB_OK);
      end;
  else
    Result := False;
  end;
end;
