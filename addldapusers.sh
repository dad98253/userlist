samba-tool user create jackieb student --given-name=Jackie --surname=Bong --job-title="Student" --department="Grade 10" --company="Island Christian Academy"
samba-tool group addmembers "ICA Students" jackieb
samba-tool user create johnk student --given-name=John --surname=Kuras --job-title="Student" --department="Grade 11" --company="Island Christian Academy"
samba-tool group addmembers "ICA Students" johnk
