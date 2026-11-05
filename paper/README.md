# Survival learning with censored data

**survlearning: A Unified R Interface for Survival Learning with Censored Data**

Pedro Luiz Ramos, Pontificia Universidad Catolica de Chile.

- [Article](survlearning.pdf): methods, software interface, numerical comparisons,
  and a hard-drive reliability application.
- [LaTeX sources](Overleaf_survlearning.zip): a flat Overleaf-ready directory
  containing one TeX file, one bibliography, figures, and the JSS style files.
- [Replication material](Replication_survlearning.zip): R scripts, baseline
  data, results, and the software version used in the numerical study.

The article documents version 0.3.5, including native neural Cox estimation,
integrated RF-SRC, Royston-Parmar and Gehan fitting components, and censored
regression, ranking, hybrid and censoring-weighted SVM objectives. It also covers
paired comparison intervals, IPCW calibration and cross-validated survival
stacking. The article has 45 pages, including references and the appendix.
Upstream-derived code is credited in `inst/COPYRIGHTS`.

The replication archive pins the 0.3.5 source used for the complete numerical
study, and `code.R` verifies its checksum. The CRAN resubmission uses the same
numerical engines, with corrections to metadata, diagnostic handling and
upstream-comparison test compatibility. Requirements and execution instructions
are in `README_REPLICATION.md`.

The application includes 29,058 eligible drives and 718 failures. The predictive
comparison uses 23,246 training drives and 5,812 held-out drives. The historical
random split illustrates the methods; it does not establish prospective validity.

Code is GPL (>= 3). Backblaze is the data source, and its separate data-use
conditions are described in `DATA.md` in the replication archive.
