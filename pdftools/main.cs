using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Shapes;
using System.Xml.Linq;
using PdfSharp.Drawing;
using PdfSharp.Pdf;
using PdfSharp.Pdf.IO;

namespace pdftools
{
    public class main
    {
        private static string TempPath = "";

        private static Image Compress(Bitmap bitmap, uint quality)
        {
            if (bitmap != null)
            {
                double q = quality / (double)100;
                int Width = Convert.ToInt32(bitmap.Width * q);
                int Height = Convert.ToInt32(bitmap.Height * q);

                Bitmap newBitmap = bitmap;
                // newBitmap.SetResolution(96, 96);
                return newBitmap.GetThumbnailImage(Width, Height, null, IntPtr.Zero);
            }
            else
            {
                throw new Exception("Please provide bitmap");
            }
        }

        static void GenerateImage(IEnumerable<string> paths, string type, string resultpath, string expand, bool iscompress, uint quality)
        {
            List<Image> images = new List<Image>();
            
            Bitmap finalImage = null;
            try
            {
                int width = 0;
                int height = 0;

                foreach (string image in paths)
                {
                    GC.Collect();

                    if (System.IO.Path.GetExtension(image) == ".pdf") return;

                    Image bitmap = iscompress ? Compress(new Bitmap(image), quality) : new Bitmap(image);
                    
                    //update the size of the final bitmap
                    if (expand == "horizontal")
                    {
                        width += bitmap.Width;
                        height = bitmap.Height > height ? bitmap.Height : height;
                    }
                    else
                    {
                        width = bitmap.Width > width ? bitmap.Width : width;
                        height += bitmap.Height;
                    }

                    images.Add(bitmap);
                }

                //create a bitmap to hold the combined image
                finalImage = new Bitmap(width, height);

                //finalImage1 = new Bitmap(width, height);

                //get a graphics object from the image so we can draw on it
                using (Graphics g = Graphics.FromImage(finalImage))
                {
                    //set background color
                    g.Clear(Color.Transparent);

                    //go through each image and draw it on the final image
                    int offX = 0, offY = 0;
                    foreach (Image image in images)
                    {
                        GC.Collect();

                        if (expand == "horizontal")
                        {
                            offY = (height - image.Height) / 2;
                        }
                        else
                        {
                            offX = (width - image.Width) / 2;
                        }

                        g.DrawImage(image,
                          new System.Drawing.Rectangle(offX, offY, image.Width, image.Height));

                        if (expand == "horizontal")
                        {
                            offX += image.Width;
                        }
                        else
                        {
                            offY += image.Height;
                        }
                    }
                }

                ImageFormat format = ImageFormat.Png;
                switch (type)
                {
                    case "png":
                        format = ImageFormat.Png;
                        break;
                    case "jpg":
                        format = ImageFormat.Jpeg;
                        break;
                    case "jpeg":
                        format = ImageFormat.Jpeg;
                        break;
                }
                Directory.CreateDirectory(System.IO.Path.GetDirectoryName(resultpath));

                if (System.IO.File.Exists(resultpath))
                    System.IO.File.Delete(resultpath);

                finalImage.Save(resultpath, format);
                GC.Collect();
            }
            catch (Exception ex)
            {
                if (finalImage != null)
                    finalImage.Dispose();

                throw ex;
            }
            finally
            {
                //clean up memory
                foreach (Image image in images)
                {
                    image.Dispose();
                    GC.Collect();
                }
            }
        }

        static void GeneratePDF(IEnumerable<string> paths, string resultpath, bool iscompress, uint quality)
        {
            using (var document = new PdfDocument())
            {
                List<string> compressed = new List<string>();
                if (iscompress)
                {
                    Directory.CreateDirectory(TempPath);

                    foreach (var path in paths)
                    {
                        string ext = System.IO.Path.GetExtension(path);
                        if (ext != ".pdf")
                        {
                            var name = System.IO.Path.GetFileName(path);
                            var save = System.IO.Path.Combine(TempPath, string.Format("Compressed_{0}", name));

                            ext = ext.Substring(1);
                            GenerateImage(new string[] { path }, ext, save, "vertical", true, quality);

                            compressed.Add(save);
                        }
                        else
                        {
                            compressed.Add(path);
                        }
                    }
                }
                else
                {
                    compressed = paths.ToList();
                }

                foreach (var path in compressed)
                {
                    if (System.IO.Path.GetExtension(path) != ".pdf")
                    {
                        PdfPage page = document.AddPage();
                        using (XImage img = XImage.FromFile(path))
                        {
                            page.Width = img.PixelWidth;
                            page.Height = img.PixelHeight;

                            XGraphics gfx = XGraphics.FromPdfPage(page);

                            gfx.DrawImage(img, 0, 0, img.PixelWidth, img.PixelHeight);
                        }
                    }
                    else
                    {
                        PdfDocument inputPDFDocument = PdfReader.Open(path, PdfDocumentOpenMode.Import);
                        foreach (PdfPage inppage in inputPDFDocument.Pages)
                        {
                            document.AddPage(inppage);
                        }
                    }

                    GC.Collect();
                }
                compressed.Clear();

                document.Options.FlateEncodeMode = PdfFlateEncodeMode.BestCompression;
                document.Options.UseFlateDecoderForJpegImages = PdfUseFlateDecoderForJpegImages.Automatic;
                document.Options.NoCompression = false;
                document.Options.CompressContentStreams = true;

                Directory.CreateDirectory(System.IO.Path.GetDirectoryName(resultpath));
                document.Save(resultpath);

                if (Directory.Exists(TempPath))
                {
                    Directory.Delete(TempPath, true);
                }

                GC.Collect();
            }
        }

        [STAThread]
        public static void Main(string[] args)
        {
            if (args.Length != 2)
            {
                return;
            }

            string root = args[0];
            TempPath = args[1];

            var imagesPath = System.IO.Path.Combine(root, "images.txt");
            var config = System.IO.Path.Combine(root, "config.txt");

            var paths = File.ReadLines(imagesPath);

            var text = File.ReadAllText(config);

            if (!text.Contains("d6bb8c8c-d255-46af-9d2d-11ee63f8d85a : ") || 
                !text.Contains("b4897706-b448-4ca2-9322-4d8cdc474ef5 : "))
            {
                Environment.Exit(1);
            }

            // lines.ElementAt(0);
            var pos = text.IndexOf("d6bb8c8c-d255-46af-9d2d-11ee63f8d85a : ") + 39;
            var nl = text.IndexOf(Environment.NewLine, pos);
            nl = nl == -1 ? text.Length : nl;
            var type = text.Substring(pos, nl - pos);

            // lines.ElementAt(1);
            pos = text.IndexOf("b4897706-b448-4ca2-9322-4d8cdc474ef5 : ") + 39;
            nl = text.IndexOf(Environment.NewLine, pos);
            nl = nl == -1 ? text.Length : nl;
            var resultpath = text.Substring(pos, nl - pos);

            // lines.ElementAtOrDefault(2)
            pos = text.IndexOf("6814ba7a-45f3-47df-907e-bbcfda04e924 : ") + 39;
            nl = text.IndexOf(Environment.NewLine, pos);
            nl = nl == -1 ? text.Length : nl;
            var expand = text.Contains("6814ba7a-45f3-47df-907e-bbcfda04e924 : ") ? 
                text.Substring(pos, nl - pos) : 
                "vertical";

            // Convert.ToBoolean(lines.ElementAtOrDefault(3));
            pos = text.IndexOf("f7d64306-d529-4972-aaa1-75d622626acc : ") + 39;
            nl = text.IndexOf(Environment.NewLine, pos);
            nl = nl == -1 ? text.Length : nl;
            var iscompress = text.Contains("6814ba7a-45f3-47df-907e-bbcfda04e924 : ") ?
                Convert.ToBoolean(text.Substring(pos, nl - pos)) :
                true;

            // Convert.ToUInt32(lines.ElementAtOrDefault(4));
            pos = text.IndexOf("0019f40c-feba-42de-875a-10d7a8626e2e : ") + 39;
            nl = text.IndexOf(Environment.NewLine, pos);
            nl = nl == -1 ? text.Length : nl;
            uint quality = text.Contains("0019f40c-feba-42de-875a-10d7a8626e2e : ") ?
                Convert.ToUInt32(text.Substring(pos, nl - pos)) :
                75;
            quality = 100 - quality;

            var extension = string.Format(".{0}", type);
            if (!resultpath.EndsWith(extension))
                resultpath += extension;

            GC.Collect();

            if (type == "pdf")
            {
                GeneratePDF(paths, resultpath, iscompress, quality);
            }
            else if(type == "png" || type == "jpg" || type == "jpeg")
            {
                GenerateImage(paths, type, resultpath, expand, iscompress, quality);
            }

            GC.Collect();
        }
    }
}
