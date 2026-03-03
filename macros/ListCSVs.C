void ListCSVs(const char* folder= "../data/Fe")
{
  TSystemDirectory dir("Fe", folder);
  TList *files = dir.GetListOfFiles();
  if (!files) { Error("ListCSVs","No files found in %s", folder); return; }

  files->Sort(); // nice ordering
  TIter next(files);
  TObject *obj;

  while ((obj = next())) {
    TString name = obj->GetName();
    if (name == "." || name == "..") continue;
    if (!name.EndsWith(".csv")) continue;

    Printf("Found CSV: %s/%s", folder, name.Data());
  }
}